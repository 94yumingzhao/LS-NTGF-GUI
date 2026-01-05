// main_window.cpp - Main Window Implementation

#include "main_window.h"
#include "parameter_widget.h"
#include "results_widget.h"
#include "log_widget.h"
#include "solver_worker.h"

#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QProgressBar>
#include <QSplitter>
#include <QFileDialog>
#include <QMessageBox>
#include <QStatusBar>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , worker_thread_(nullptr)
    , solver_worker_(nullptr)
    , is_running_(false)
    , total_runtime_(0.0) {
    SetupUi();
    SetupMenuBar();
    SetupConnections();
    UpdateUiState(false);

    setWindowTitle(QString::fromUtf8("生产计划优化器"));
    resize(900, 700);
}

MainWindow::~MainWindow() {
    if (worker_thread_) {
        worker_thread_->quit();
        worker_thread_->wait();
    }
}

void MainWindow::SetupUi() {
    auto* central = new QWidget(this);
    auto* main_layout = new QVBoxLayout(central);
    main_layout->setSpacing(8);
    main_layout->setContentsMargins(10, 10, 10, 10);

    // Top row: File selection + Parameters
    auto* top_layout = new QHBoxLayout();

    // File selection group
    file_group_ = new QGroupBox(QString::fromUtf8("文件选择"), this);
    auto* file_layout = new QVBoxLayout(file_group_);

    auto* browse_layout = new QHBoxLayout();
    browse_button_ = new QPushButton(QString::fromUtf8("浏览..."), this);
    browse_button_->setFixedWidth(80);
    file_path_edit_ = new QLineEdit(this);
    file_path_edit_->setReadOnly(true);
    file_path_edit_->setPlaceholderText(QString::fromUtf8("选择CSV数据文件..."));
    browse_layout->addWidget(browse_button_);
    browse_layout->addWidget(file_path_edit_);

    file_info_label_ = new QLabel(QString::fromUtf8("订单数: --  周期数: --  流向数: --  分组数: --"), this);
    file_info_label_->setStyleSheet("color: gray;");

    file_layout->addLayout(browse_layout);
    file_layout->addWidget(file_info_label_);

    // Parameters widget
    param_widget_ = new ParameterWidget(this);

    top_layout->addWidget(file_group_, 1);
    top_layout->addWidget(param_widget_, 1);
    main_layout->addLayout(top_layout);

    // Run control row
    auto* run_layout = new QHBoxLayout();
    start_button_ = new QPushButton(QString::fromUtf8("开始优化"), this);
    start_button_->setMinimumHeight(36);
    start_button_->setStyleSheet("font-weight: bold;");
    cancel_button_ = new QPushButton(QString::fromUtf8("取消"), this);
    cancel_button_->setMinimumHeight(36);
    status_label_ = new QLabel(QString::fromUtf8("状态: 就绪"), this);

    run_layout->addWidget(start_button_);
    run_layout->addWidget(cancel_button_);
    run_layout->addStretch();
    run_layout->addWidget(status_label_);
    main_layout->addLayout(run_layout);

    // Progress group
    progress_group_ = new QGroupBox(QString::fromUtf8("求解进度"), this);
    auto* progress_layout = new QGridLayout(progress_group_);

    const char* stage_names[] = {
        "\xe8\xae\xa2\xe5\x8d\x95\xe5\x90\x88\xe5\xb9\xb6:",      // 订单合并:
        "\xe9\x98\xb6\xe6\xae\xb5 1 (\xe8\xae\xbe\xe7\xbd\xae):", // 阶段 1 (设置):
        "\xe9\x98\xb6\xe6\xae\xb5 2 (\xe7\xbb\x93\xe8\xbd\xac):", // 阶段 2 (结转):
        "\xe9\x98\xb6\xe6\xae\xb5 3 (\xe6\x9c\x80\xe7\xbb\x88):"  // 阶段 3 (最终):
    };
    for (int i = 0; i < 4; ++i) {
        stage_labels_[i] = new QLabel(QString::fromUtf8(stage_names[i]), this);
        progress_bars_[i] = new QProgressBar(this);
        progress_bars_[i]->setRange(0, 100);
        progress_bars_[i]->setValue(0);
        progress_bars_[i]->setTextVisible(true);
        time_labels_[i] = new QLabel("--", this);
        time_labels_[i]->setFixedWidth(60);
        time_labels_[i]->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

        progress_layout->addWidget(stage_labels_[i], i, 0);
        progress_layout->addWidget(progress_bars_[i], i, 1);
        progress_layout->addWidget(time_labels_[i], i, 2);
    }
    progress_layout->setColumnStretch(1, 1);
    main_layout->addWidget(progress_group_);

    // Results and Log (splitter)
    auto* splitter = new QSplitter(Qt::Horizontal, this);
    results_widget_ = new ResultsWidget(this);
    log_widget_ = new LogWidget(this);

    splitter->addWidget(results_widget_);
    splitter->addWidget(log_widget_);
    splitter->setStretchFactor(0, 2);
    splitter->setStretchFactor(1, 1);
    main_layout->addWidget(splitter, 1);

    // Bottom row: Export + Total time
    auto* bottom_layout = new QHBoxLayout();
    export_button_ = new QPushButton(QString::fromUtf8("导出结果..."), this);
    total_time_label_ = new QLabel(QString::fromUtf8("总耗时: --"), this);
    total_time_label_->setStyleSheet("font-weight: bold;");

    bottom_layout->addWidget(export_button_);
    bottom_layout->addStretch();
    bottom_layout->addWidget(total_time_label_);
    main_layout->addLayout(bottom_layout);

    setCentralWidget(central);

    // Status bar
    statusBar()->showMessage(QString::fromUtf8("就绪"));
}

void MainWindow::SetupMenuBar() {
    auto* file_menu = menuBar()->addMenu(QString::fromUtf8("文件(&F)"));

    auto* open_action = new QAction(QString::fromUtf8("打开数据文件(&O)..."), this);
    open_action->setShortcut(QKeySequence::Open);
    connect(open_action, &QAction::triggered, this, &MainWindow::OnBrowseFile);
    file_menu->addAction(open_action);

    auto* export_action = new QAction(QString::fromUtf8("导出结果(&E)..."), this);
    export_action->setShortcut(QKeySequence::Save);
    connect(export_action, &QAction::triggered, this, &MainWindow::OnExportResults);
    file_menu->addAction(export_action);

    file_menu->addSeparator();

    auto* exit_action = new QAction(QString::fromUtf8("退出(&X)"), this);
    exit_action->setShortcut(QKeySequence::Quit);
    connect(exit_action, &QAction::triggered, this, &QWidget::close);
    file_menu->addAction(exit_action);

    auto* help_menu = menuBar()->addMenu(QString::fromUtf8("帮助(&H)"));
    auto* about_action = new QAction(QString::fromUtf8("关于(&A)"), this);
    connect(about_action, &QAction::triggered, [this]() {
        QMessageBox::about(this, QString::fromUtf8("关于"),
            QString::fromUtf8(
                "生产计划优化器 GUI\n\n"
                "版本 1.0.0\n\n"
                "基于 Qt6 的 LS-NTGF-RR 求解器界面"));
    });
    help_menu->addAction(about_action);
}

void MainWindow::SetupConnections() {
    connect(browse_button_, &QPushButton::clicked, this, &MainWindow::OnBrowseFile);
    connect(start_button_, &QPushButton::clicked, this, &MainWindow::OnStartOptimization);
    connect(cancel_button_, &QPushButton::clicked, this, &MainWindow::OnCancelOptimization);
    connect(export_button_, &QPushButton::clicked, this, &MainWindow::OnExportResults);

    // Setup worker thread
    worker_thread_ = new QThread(this);
    solver_worker_ = new SolverWorker();
    solver_worker_->moveToThread(worker_thread_);

    connect(this, &MainWindow::StartSolver, solver_worker_, &SolverWorker::RunOptimization);
    connect(solver_worker_, &SolverWorker::DataLoaded, this, &MainWindow::OnDataLoaded);
    connect(solver_worker_, &SolverWorker::StageStarted, this, &MainWindow::OnStageStarted);
    connect(solver_worker_, &SolverWorker::StageProgress, this, &MainWindow::OnStageProgress);
    connect(solver_worker_, &SolverWorker::StageCompleted, this, &MainWindow::OnStageCompleted);
    connect(solver_worker_, &SolverWorker::OptimizationFinished, this, &MainWindow::OnOptimizationFinished);
    connect(solver_worker_, &SolverWorker::LogMessage, this, &MainWindow::OnLogMessage);

    connect(worker_thread_, &QThread::finished, solver_worker_, &QObject::deleteLater);

    worker_thread_->start();
}

void MainWindow::UpdateUiState(bool is_running) {
    is_running_ = is_running;
    browse_button_->setEnabled(!is_running);
    param_widget_->setEnabled(!is_running);
    start_button_->setEnabled(!is_running && !current_file_path_.isEmpty());
    cancel_button_->setEnabled(is_running);
    export_button_->setEnabled(!is_running && results_widget_->HasResults());

    status_label_->setText(is_running ? QString::fromUtf8("状态: 运行中...") : QString::fromUtf8("状态: 就绪"));
}

void MainWindow::ResetProgress() {
    for (int i = 0; i < 4; ++i) {
        progress_bars_[i]->setValue(0);
        time_labels_[i]->setText("--");
    }
    results_widget_->ClearResults();
    log_widget_->ClearLog();
    total_runtime_ = 0.0;
    total_time_label_->setText(QString::fromUtf8("总耗时: --"));
}

void MainWindow::OnBrowseFile() {
    QString path = QFileDialog::getOpenFileName(this,
        QString::fromUtf8("选择数据文件"),
        "D:/YM-Code/LS-NTGF-Data-Cap/data",
        QString::fromUtf8("CSV 文件 (*.csv);;所有文件 (*)"));

    if (!path.isEmpty()) {
        current_file_path_ = path;
        file_path_edit_->setText(path);
        file_info_label_->setText(QString::fromUtf8("正在加载文件信息..."));
        file_info_label_->setStyleSheet("color: gray;");

        // Quick file info preview (will be updated when data loads)
        start_button_->setEnabled(true);
        log_widget_->AppendLog(QString::fromUtf8("已选择文件: ") + path);
    }
}

void MainWindow::OnStartOptimization() {
    if (current_file_path_.isEmpty()) {
        QMessageBox::warning(this, QString::fromUtf8("错误"),
            QString::fromUtf8("请先选择数据文件"));
        return;
    }

    ResetProgress();
    UpdateUiState(true);

    // Set parameters
    solver_worker_->SetDataPath(current_file_path_);
    solver_worker_->SetParameters(
        param_widget_->GetRuntimeLimit(),
        param_widget_->GetUPenalty(),
        param_widget_->GetBPenalty(),
        param_widget_->GetBigOrderThreshold()
    );

    log_widget_->AppendLog(QString::fromUtf8("开始优化..."));
    statusBar()->showMessage(QString::fromUtf8("正在优化..."));

    emit StartSolver();
}

void MainWindow::OnCancelOptimization() {
    if (solver_worker_) {
        solver_worker_->RequestCancel();
        log_widget_->AppendLog(QString::fromUtf8("正在取消..."));
    }
}

void MainWindow::OnExportResults() {
    QString path = QFileDialog::getSaveFileName(this,
        QString::fromUtf8("导出结果"),
        "results.csv",
        QString::fromUtf8("CSV 文件 (*.csv);;所有文件 (*)"));

    if (!path.isEmpty()) {
        if (results_widget_->ExportToCsv(path)) {
            log_widget_->AppendLog(QString::fromUtf8("结果已导出: ") + path);
            QMessageBox::information(this, QString::fromUtf8("导出"),
                QString::fromUtf8("结果导出成功"));
        } else {
            QMessageBox::warning(this, QString::fromUtf8("导出错误"),
                QString::fromUtf8("导出结果失败"));
        }
    }
}

void MainWindow::OnDataLoaded(int items, int periods, int flows, int groups) {
    QString info = QString::fromUtf8("订单数: %1  周期数: %2  流向数: %3  分组数: %4")
        .arg(items).arg(periods).arg(flows).arg(groups);
    file_info_label_->setText(info);
    file_info_label_->setStyleSheet("color: black;");
    log_widget_->AppendLog(QString::fromUtf8("数据已加载: %1 个订单, %2 个周期").arg(items).arg(periods));
}

void MainWindow::OnStageStarted(int stage, const QString& name) {
    if (stage >= 0 && stage < 4) {
        progress_bars_[stage]->setValue(0);
        time_labels_[stage]->setText("...");
    }
    log_widget_->AppendLog(QString::fromUtf8("开始: ") + name);
}

void MainWindow::OnStageProgress(int stage, int percent, double elapsed) {
    if (stage >= 0 && stage < 4) {
        progress_bars_[stage]->setValue(percent);
        time_labels_[stage]->setText(QString("%1s").arg(elapsed, 0, 'f', 1));
    }
}

void MainWindow::OnStageCompleted(int stage, double objective, double runtime, double gap) {
    if (stage >= 0 && stage < 4) {
        progress_bars_[stage]->setValue(100);
        time_labels_[stage]->setText(QString("%1s").arg(runtime, 0, 'f', 2));
    }

    // Update results widget (stages 1-3 map to result rows 0-2)
    if (stage >= 1 && stage <= 3) {
        results_widget_->SetStageResult(stage - 1, objective, runtime, gap);
    }

    total_runtime_ += runtime;
    total_time_label_->setText(QString::fromUtf8("总耗时: %1s").arg(total_runtime_, 0, 'f', 2));

    log_widget_->AppendLog(QString::fromUtf8("阶段 %1 完成: 目标值=%2, 耗时=%3s, Gap=%4%")
        .arg(stage).arg(objective, 0, 'f', 2).arg(runtime, 0, 'f', 3).arg(gap * 100, 0, 'f', 4));
}

void MainWindow::OnOptimizationFinished(bool success, const QString& message) {
    UpdateUiState(false);

    if (success) {
        statusBar()->showMessage(QString::fromUtf8("优化完成"));
        log_widget_->AppendLog(QString::fromUtf8("优化完成: ") + message);
    } else {
        statusBar()->showMessage(QString::fromUtf8("优化已停止"));
        log_widget_->AppendLog(QString::fromUtf8("优化已停止: ") + message);
        QMessageBox::warning(this, QString::fromUtf8("优化"), message);
    }
}

void MainWindow::OnLogMessage(const QString& message) {
    log_widget_->AppendLog(message);
}
