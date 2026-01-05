// solver_worker.cpp - Background Solver Worker Thread Implementation

#include "solver_worker.h"
#include "optimizer.h"  // From LS-NTGF-RR core

#include <QTimer>
#include <QThread>

SolverWorker::SolverWorker(QObject* parent)
    : QObject(parent)
    , runtime_limit_(30.0)
    , u_penalty_(10000)
    , b_penalty_(100)
    , big_order_threshold_(1000.0)
    , cancel_requested_(false)
    , values_(nullptr)
    , lists_(nullptr) {
}

SolverWorker::~SolverWorker() {
    delete values_;
    delete lists_;
}

void SolverWorker::SetDataPath(const QString& path) {
    data_path_ = path;
}

void SolverWorker::SetParameters(double runtime_limit, int u_penalty,
                                  int b_penalty, double big_order_threshold) {
    runtime_limit_ = runtime_limit;
    u_penalty_ = u_penalty;
    b_penalty_ = b_penalty;
    big_order_threshold_ = big_order_threshold;
}

void SolverWorker::RequestCancel() {
    cancel_requested_ = true;
}

void SolverWorker::EmitProgress(int stage, std::chrono::steady_clock::time_point start) {
    auto now = std::chrono::steady_clock::now();
    double elapsed = std::chrono::duration<double>(now - start).count();
    int percent = std::min(99, static_cast<int>(elapsed / runtime_limit_ * 100));
    emit StageProgress(stage, percent, elapsed);
}

void SolverWorker::RunOptimization() {
    cancel_requested_ = false;

    // Allocate solver data structures
    delete values_;
    delete lists_;
    values_ = new AllValues();
    lists_ = new AllLists();

    try {
        // Stage 0: Load data
        emit StageStarted(0, "Loading data");
        emit LogMessage("Reading data file: " + data_path_);

        auto load_start = std::chrono::steady_clock::now();
        ReadData(*values_, *lists_, data_path_.toStdString());

        auto load_end = std::chrono::steady_clock::now();
        double load_time = std::chrono::duration<double>(load_end - load_start).count();

        emit DataLoaded(values_->number_of_items, values_->number_of_periods,
                        values_->number_of_flows, values_->number_of_groups);
        emit StageProgress(0, 100, load_time);
        emit StageCompleted(0, 0, load_time, 0);

        if (cancel_requested_) {
            emit OptimizationFinished(false, "Cancelled by user");
            return;
        }

        // Apply user parameters
        values_->cpx_runtime_limit = runtime_limit_;
        values_->u_penalty = u_penalty_;
        values_->b_penalty = b_penalty_;
        values_->big_order_threshold = big_order_threshold_;

        emit LogMessage(QString("Parameters: CPLEX limit=%1s, U=%2, B=%3")
            .arg(runtime_limit_).arg(u_penalty_).arg(b_penalty_));

        // Stage 0.5: Order merge
        emit StageStarted(0, "Merging orders");
        emit LogMessage("Merging small orders...");

        auto merge_start = std::chrono::steady_clock::now();
        int original_items = values_->number_of_items;
        UpdateBigOrderFG(*values_, *lists_);
        auto merge_end = std::chrono::steady_clock::now();
        double merge_time = std::chrono::duration<double>(merge_end - merge_start).count();

        emit LogMessage(QString("Orders merged: %1 -> %2")
            .arg(original_items).arg(values_->number_of_items));
        emit StageProgress(0, 100, merge_time);

        if (cancel_requested_) {
            emit OptimizationFinished(false, "Cancelled by user");
            return;
        }

        // Stage 1: Setup optimization
        emit StageStarted(1, "Stage 1 - Setup Optimization");
        emit LogMessage("Running Stage 1 (Setup structure)...");

        auto stage1_start = std::chrono::steady_clock::now();

        // Progress timer for Stage 1
        QTimer progress_timer;
        connect(&progress_timer, &QTimer::timeout, [this, stage1_start]() {
            EmitProgress(1, stage1_start);
        });
        progress_timer.start(500);

        SolveStep1(*values_, *lists_);

        progress_timer.stop();

        emit StageProgress(1, 100, values_->result_step1.runtime);
        emit StageCompleted(1, values_->result_step1.objective,
                           values_->result_step1.runtime, values_->result_step1.gap);

        if (cancel_requested_) {
            emit OptimizationFinished(false, "Cancelled by user");
            return;
        }

        // Stage 2: Carryover optimization
        emit StageStarted(2, "Stage 2 - Carryover Optimization");
        emit LogMessage("Running Stage 2 (Carryover)...");

        auto stage2_start = std::chrono::steady_clock::now();

        QTimer progress_timer2;
        connect(&progress_timer2, &QTimer::timeout, [this, stage2_start]() {
            EmitProgress(2, stage2_start);
        });
        progress_timer2.start(500);

        SolveStep2(*values_, *lists_);

        progress_timer2.stop();

        emit StageProgress(2, 100, values_->result_step2.runtime);
        emit StageCompleted(2, values_->result_step2.objective,
                           values_->result_step2.runtime, values_->result_step2.gap);

        if (cancel_requested_) {
            emit OptimizationFinished(false, "Cancelled by user");
            return;
        }

        // Stage 3: Final optimization
        emit StageStarted(3, "Stage 3 - Final Optimization");
        emit LogMessage("Running Stage 3 (Final plan)...");

        auto stage3_start = std::chrono::steady_clock::now();

        QTimer progress_timer3;
        connect(&progress_timer3, &QTimer::timeout, [this, stage3_start]() {
            EmitProgress(3, stage3_start);
        });
        progress_timer3.start(500);

        SolveStep3(*values_, *lists_);

        progress_timer3.stop();

        emit StageProgress(3, 100, values_->result_step3.runtime);
        emit StageCompleted(3, values_->result_step3.objective,
                           values_->result_step3.runtime, values_->result_step3.gap);

        // Success
        double total_time = values_->result_step1.runtime +
                           values_->result_step2.runtime +
                           values_->result_step3.runtime;
        emit LogMessage(QString("Optimization completed. Total time: %1s").arg(total_time, 0, 'f', 3));
        emit OptimizationFinished(true, QString("Completed in %1s").arg(total_time, 0, 'f', 3));

    } catch (const std::exception& e) {
        emit LogMessage(QString("Error: %1").arg(e.what()));
        emit OptimizationFinished(false, QString("Error: %1").arg(e.what()));
    } catch (...) {
        emit LogMessage("Unknown error occurred");
        emit OptimizationFinished(false, "Unknown error occurred");
    }
}
