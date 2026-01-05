// solver_worker.h - Background Solver Worker Thread

#ifndef SOLVER_WORKER_H_
#define SOLVER_WORKER_H_

#include <QObject>
#include <QString>
#include <atomic>
#include <chrono>

// Forward declarations for solver data structures
struct AllValues;
struct AllLists;

class SolverWorker : public QObject {
    Q_OBJECT

public:
    explicit SolverWorker(QObject* parent = nullptr);
    ~SolverWorker() override;

    void SetDataPath(const QString& path);
    void SetParameters(double runtime_limit, int u_penalty,
                       int b_penalty, double big_order_threshold);

public slots:
    void RunOptimization();
    void RequestCancel();

signals:
    void DataLoaded(int items, int periods, int flows, int groups);
    void StageStarted(int stage, const QString& name);
    void StageProgress(int stage, int percent, double elapsed);
    void StageCompleted(int stage, double objective, double runtime, double gap);
    void OptimizationFinished(bool success, const QString& message);
    void LogMessage(const QString& message);

private:
    void EmitProgress(int stage, std::chrono::steady_clock::time_point start);

    QString data_path_;
    double runtime_limit_;
    int u_penalty_;
    int b_penalty_;
    double big_order_threshold_;

    std::atomic<bool> cancel_requested_;

    // Solver data (owned by worker)
    AllValues* values_;
    AllLists* lists_;
};

#endif  // SOLVER_WORKER_H_
