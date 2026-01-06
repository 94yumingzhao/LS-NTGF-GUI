// main_window.cpp - Main Window Implementation
// Layout: Left sidebar + Right log area

#include "main_window.h"
#include "parameter_widget.h"
#include "results_widget.h"
#include "log_widget.h"
#include "solver_worker.h"
#include "generator_widget.h"
#include "generator_worker.h"

#include <QMenuBar>
#include <QTabWidget>
#include <QMenu>
#include <QAction>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSplitter>
#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>
#include <QStatusBar>
#include <QFile>
#include <QTextStream>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , solver_thread_(nullptr)
    , solver_worker_(nullptr)
    , generator_thread_(nullptr)
    , generator_worker_(nullptr)
    , is_running_(false)
    , total_runtime_(0.0) {
    SetupUi();
    SetupMenuBar();
    SetupConnections();
    UpdateUiState(false);

    setWindowTitle(QString::fromUtf8("LS-NTGF"));
    resize(1000, 700);
}

MainWindow::~MainWindow() {
    if (solver_thread_) {
        solver_thread_->quit();
        solver_thread_->wait();
    }
    if (generator_thread_) {
        generator_thread_->quit();
        generator_thread_->wait();
    }
}

void MainWindow::SetupUi() {
    auto* central = new QWidget(this);
    auto* main_layout = new QHBoxLayout(central);
    main_layout->setSpacing(8);
    main_layout->setContentsMargins(8, 8, 8, 8);

    // Main splitter: left sidebar + right log
    main_splitter_ = new QSplitter(Qt::Horizontal, this);

    // ========== Left Sidebar ==========
    auto* sidebar = new QWidget(this);
    auto* sidebar_layout = new QVBoxLayout(sidebar);
    sidebar_layout->setSpacing(8);
    sidebar_layout->setContentsMargins(0, 0, 0, 0);

    // --- Mode Tabs (Solver / Generator) ---
    mode_tabs_ = new QTabWidget(this);

    // ===== Solver Tab =====
    auto* solver_tab = new QWidget();
    auto* solver_layout = new QVBoxLayout(solver_tab);
    solver_layout->setSpacing(8);
    solver_layout->setContentsMargins(4, 4, 4, 4);

    // File Selection Group
    file_group_ = new QGroupBox(QString::fromUtf8(""), solver_tab);
    auto* file_layout = new QVBoxLayout(file_group_);
    file_layout->setSpacing(4);

    auto* browse_layout = new QHBoxLayout();
    browse_button_ = new QPushButton(QString::fromUtf8("..."), this);
    browse_button_->setFixedWidth(32);
    browse_button_->setToolTip(QString::fromUtf8(""));
    file_path_edit_ = new QLineEdit(this);
    file_path_edit_->setReadOnly(true);
    file_path_edit_->setPlaceholderText(QString::fromUtf8("CSV..."));
    browse_layout->addWidget(browse_button_);
    browse_layout->addWidget(file_path_edit_);
    file_layout->addLayout(browse_layout);

    file_info_label_ = new QLabel(QString::fromUtf8("--"), this);
    file_info_label_->setStyleSheet("color: gray; font-size: 9pt;");
    file_layout->addWidget(file_info_label_);

    solver_layout->addWidget(file_group_);

    // Parameters Widget
    param_widget_ = new ParameterWidget(solver_tab);
    solver_layout->addWidget(param_widget_);

    // Control Group
    control_group_ = new QGroupBox(solver_tab);
    auto* control_layout = new QVBoxLayout(control_group_);
    control_layout->setSpacing(4);

    auto* button_layout = new QHBoxLayout();
    start_button_ = new QPushButton(QString::fromUtf8("Run"), this);
    start_button_->setMinimumHeight(32);
    start_button_->setStyleSheet("font-weight: bold;");
    cancel_button_ = new QPushButton(QString::fromUtf8("Cancel"), this);
    cancel_button_->setMinimumHeight(32);
    button_layout->addWidget(start_button_);
    button_layout->addWidget(cancel_button_);
    control_layout->addLayout(button_layout);

    status_label_ = new QLabel(QString::fromUtf8("Ready"), this);
    status_label_->setAlignment(Qt::AlignCenter);
    control_layout->addWidget(status_label_);

    solver_layout->addWidget(control_group_);

    // Results Widget
    results_widget_ = new ResultsWidget(solver_tab);
    solver_layout->addWidget(results_widget_);

    // Export Button
    export_button_ = new QPushButton(QString::fromUtf8("Export Log..."), solver_tab);
    solver_layout->addWidget(export_button_);

    solver_layout->addStretch();

    // ===== Generator Tab =====
    generator_widget_ = new GeneratorWidget();

    // Add tabs
    mode_tabs_->addTab(solver_tab, "Solver");
    mode_tabs_->addTab(generator_widget_, "Generator");

    sidebar_layout->addWidget(mode_tabs_);

    // ========== Right Log Area ==========
    log_widget_ = new LogWidget(this);

    // Add to splitter
    main_splitter_->addWidget(sidebar);
    main_splitter_->addWidget(log_widget_);
    main_splitter_->setStretchFactor(0, 0);  // Sidebar: fixed
    main_splitter_->setStretchFactor(1, 1);  // Log: stretch
    main_splitter_->setSizes({280, 720});

    main_layout->addWidget(main_splitter_);
    setCentralWidget(central);

    // Status bar
    statusBar()->showMessage(QString::fromUtf8("Ready"));
}

void MainWindow::SetupMenuBar() {
    auto* file_menu = menuBar()->addMenu(QString::fromUtf8("&File"));

    auto* open_action = new QAction(QString::fromUtf8("&Open..."), this);
    open_action->setShortcut(QKeySequence::Open);
    connect(open_action, &QAction::triggered, this, &MainWindow::OnBrowseFile);
    file_menu->addAction(open_action);

    auto* export_action = new QAction(QString::fromUtf8("&Export Log..."), this);
    export_action->setShortcut(QKeySequence::Save);
    connect(export_action, &QAction::triggered, this, &MainWindow::OnExportLog);
    file_menu->addAction(export_action);

    file_menu->addSeparator();

    auto* exit_action = new QAction(QString::fromUtf8("E&xit"), this);
    exit_action->setShortcut(QKeySequence::Quit);
    connect(exit_action, &QAction::triggered, this, &QWidget::close);
    file_menu->addAction(exit_action);

    auto* help_menu = menuBar()->addMenu(QString::fromUtf8("&Help"));
    auto* about_action = new QAction(QString::fromUtf8("&About"), this);
    connect(about_action, &QAction::triggered, [this]() {
        QMessageBox::about(this, QString::fromUtf8("About"),
            QString::fromUtf8(
                "LS-NTGF GUI\n\n"
                "Version 2.0.0\n\n"
                "Algorithms:\n"
                "  RF  - Relax-and-Fix\n"
                "  RFO - RF + Fix-and-Optimize\n"
                "  RR  - PP-GCB (3-stage)"));
    });
    help_menu->addAction(about_action);
}

void MainWindow::SetupConnections() {
    connect(browse_button_, &QPushButton::clicked, this, &MainWindow::OnBrowseFile);
    connect(start_button_, &QPushButton::clicked, this, &MainWindow::OnStartOptimization);
    connect(cancel_button_, &QPushButton::clicked, this, &MainWindow::OnCancelOptimization);
    connect(export_button_, &QPushButton::clicked, this, &MainWindow::OnExportLog);

    // Algorithm change
    connect(param_widget_, &ParameterWidget::AlgorithmChanged,
            this, &MainWindow::OnAlgorithmChanged);

    // Setup solver worker thread
    solver_thread_ = new QThread(this);
    solver_worker_ = new SolverWorker();
    solver_worker_->moveToThread(solver_thread_);

    connect(this, &MainWindow::StartSolver, solver_worker_, &SolverWorker::RunOptimization);
    connect(solver_worker_, &SolverWorker::DataLoaded, this, &MainWindow::OnDataLoaded);
    connect(solver_worker_, &SolverWorker::OrdersMerged, this, &MainWindow::OnOrdersMerged);
    connect(solver_worker_, &SolverWorker::MergeSkipped, this, &MainWindow::OnMergeSkipped);
    connect(solver_worker_, &SolverWorker::StageStarted, this, &MainWindow::OnStageStarted);
    connect(solver_worker_, &SolverWorker::StageCompleted, this, &MainWindow::OnStageCompleted);
    connect(solver_worker_, &SolverWorker::OptimizationFinished, this, &MainWindow::OnOptimizationFinished);
    connect(solver_worker_, &SolverWorker::LogMessage, this, &MainWindow::OnLogMessage);

    connect(solver_thread_, &QThread::finished, solver_worker_, &QObject::deleteLater);
    solver_thread_->start();

    // Setup generator worker thread
    generator_thread_ = new QThread(this);
    generator_worker_ = new GeneratorWorker();
    generator_worker_->moveToThread(generator_thread_);

    connect(generator_widget_, &GeneratorWidget::GenerateRequested,
            this, &MainWindow::OnGenerateRequested);
    connect(generator_worker_, &GeneratorWorker::GenerationStarted,
            this, &MainWindow::OnGenerationStarted);
    connect(generator_worker_, &GeneratorWorker::InstanceGenerated,
            this, &MainWindow::OnInstanceGenerated);
    connect(generator_worker_, &GeneratorWorker::GenerationFinished,
            this, &MainWindow::OnGenerationFinished);
    connect(generator_worker_, &GeneratorWorker::LogMessage,
            this, &MainWindow::OnLogMessage);

    connect(generator_thread_, &QThread::finished, generator_worker_, &QObject::deleteLater);
    generator_thread_->start();

    // Initialize results widget with current algorithm
    OnAlgorithmChanged(param_widget_->GetAlgorithmIndex());
}

void MainWindow::UpdateUiState(bool is_running) {
    is_running_ = is_running;
    browse_button_->setEnabled(!is_running);
    param_widget_->setEnabled(!is_running);
    start_button_->setEnabled(!is_running && !current_file_path_.isEmpty());
    cancel_button_->setEnabled(is_running);
    export_button_->setEnabled(!is_running);

    status_label_->setText(is_running ?
        QString::fromUtf8("Running...") :
        QString::fromUtf8("Ready"));
}

void MainWindow::ResetState() {
    results_widget_->ClearResults();
    log_widget_->ClearLog();
    total_runtime_ = 0.0;
}

void MainWindow::OnBrowseFile() {
    QString path = QFileDialog::getOpenFileName(this,
        QString::fromUtf8("Open Data File"),
        "D:/YM-Code/LS-NTGF-Data-Cap/data",
        QString::fromUtf8("CSV (*.csv);;All Files (*)"));

    if (!path.isEmpty()) {
        current_file_path_ = path;
        // Show only filename in the edit
        QFileInfo fi(path);
        file_path_edit_->setText(fi.fileName());
        file_path_edit_->setToolTip(path);
        file_info_label_->setText(QString::fromUtf8("Loading..."));
        file_info_label_->setStyleSheet("color: gray; font-size: 9pt;");

        start_button_->setEnabled(true);
        log_widget_->AppendLog(QString::fromUtf8("File: ") + path);
    }
}

void MainWindow::OnStartOptimization() {
    if (current_file_path_.isEmpty()) {
        QMessageBox::warning(this, QString::fromUtf8("Error"),
            QString::fromUtf8("Please select a data file first"));
        return;
    }

    ResetState();
    UpdateUiState(true);

    // Set algorithm and parameters
    int algo_idx = param_widget_->GetAlgorithmIndex();
    AlgorithmType algo = static_cast<AlgorithmType>(algo_idx);
    solver_worker_->SetAlgorithm(algo);
    solver_worker_->SetDataPath(current_file_path_);
    solver_worker_->SetParameters(
        param_widget_->GetRuntimeLimit(),
        param_widget_->GetUPenalty(),
        param_widget_->GetBPenalty(),
        param_widget_->GetMergeEnabled(),
        param_widget_->GetBigOrderThreshold()
    );

    QString algo_names[] = {"RF", "RFO", "RR"};
    log_widget_->AppendLog(QString::fromUtf8("Starting optimization (Algorithm: %1)...")
        .arg(algo_names[algo_idx]));
    statusBar()->showMessage(QString::fromUtf8("Optimizing..."));

    emit StartSolver();
}

void MainWindow::OnCancelOptimization() {
    if (solver_worker_) {
        solver_worker_->RequestCancel();
        log_widget_->AppendLog(QString::fromUtf8("Cancelling..."));
    }
}

void MainWindow::OnExportLog() {
    QString path = QFileDialog::getSaveFileName(this,
        QString::fromUtf8("Export Log"),
        "log.txt",
        QString::fromUtf8("Text Files (*.txt);;All Files (*)"));

    if (!path.isEmpty()) {
        QFile file(path);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream out(&file);
            out << log_widget_->GetLogText();
            file.close();
            log_widget_->AppendLog(QString::fromUtf8("Log exported: ") + path);
            QMessageBox::information(this, QString::fromUtf8("Export"),
                QString::fromUtf8("Log exported successfully"));
        } else {
            QMessageBox::warning(this, QString::fromUtf8("Export Error"),
                QString::fromUtf8("Failed to export log"));
        }
    }
}

void MainWindow::OnAlgorithmChanged(int index) {
    AlgorithmType algo = static_cast<AlgorithmType>(index);
    results_widget_->SetAlgorithmType(algo);
}

void MainWindow::OnDataLoaded(int items, int periods, int flows, int groups) {
    QString info = QString::fromUtf8("Items:%1 Periods:%2 Flows:%3 Groups:%4")
        .arg(items).arg(periods).arg(flows).arg(groups);
    file_info_label_->setText(info);
    file_info_label_->setStyleSheet("color: black; font-size: 9pt;");
    log_widget_->AppendLog(QString::fromUtf8("Data loaded: %1 items, %2 periods")
        .arg(items).arg(periods));
}

void MainWindow::OnOrdersMerged(int original, int merged) {
    log_widget_->AppendLog(QString::fromUtf8("Orders merged: %1 -> %2")
        .arg(original).arg(merged));
    results_widget_->SetMergeInfo(original, merged);
}

void MainWindow::OnMergeSkipped() {
    log_widget_->AppendLog(QString::fromUtf8("Order merge: Skipped"));
    results_widget_->SetMergeSkipped();
}

void MainWindow::OnStageStarted(int stage, const QString& name) {
    log_widget_->AppendLog(QString::fromUtf8("Started: ") + name);
}

void MainWindow::OnStageCompleted(int stage, double objective, double runtime, double gap) {
    total_runtime_ += runtime;

    // Update results widget
    results_widget_->SetStageResult(stage, objective, runtime, gap);

    log_widget_->AppendLog(QString::fromUtf8("Stage %1 done: Obj=%2, Time=%3s, Gap=%4%")
        .arg(stage).arg(objective, 0, 'f', 2).arg(runtime, 0, 'f', 3).arg(gap * 100, 0, 'f', 4));
}

void MainWindow::OnOptimizationFinished(bool success, const QString& message) {
    UpdateUiState(false);

    results_widget_->SetTotalRuntime(total_runtime_);

    if (success) {
        statusBar()->showMessage(QString::fromUtf8("Optimization completed"));
        log_widget_->AppendLog(QString::fromUtf8("Completed: ") + message);
    } else {
        statusBar()->showMessage(QString::fromUtf8("Optimization stopped"));
        log_widget_->AppendLog(QString::fromUtf8("Stopped: ") + message);
        QMessageBox::warning(this, QString::fromUtf8("Optimization"), message);
    }
}

void MainWindow::OnLogMessage(const QString& message) {
    log_widget_->AppendLog(message);
}

// ============================================================================
// Generator Slots
// ============================================================================

void MainWindow::OnGenerateRequested(const GeneratorConfig& config) {
    log_widget_->ClearLog();
    log_widget_->AppendLog("Starting instance generation...");

    generator_worker_->SetConfig(config);

    // Use QMetaObject to invoke across threads
    QMetaObject::invokeMethod(generator_worker_, "RunGeneration", Qt::QueuedConnection);

    statusBar()->showMessage("Generating...");
}

void MainWindow::OnGenerationStarted(int count) {
    log_widget_->AppendLog(QString("Generating %1 instance(s)...").arg(count));
}

void MainWindow::OnInstanceGenerated(int index, const QString& filename) {
    log_widget_->AppendLog(QString("[%1] Generated: %2").arg(index).arg(filename));
}

void MainWindow::OnGenerationFinished(bool success, const QString& message,
                                       const QStringList& files) {
    if (success) {
        statusBar()->showMessage("Generation completed");
        log_widget_->AppendLog("Generation completed: " + message);

        // Offer to load the first generated file
        if (!files.isEmpty()) {
            QString first_file = files.first();
            if (!first_file.startsWith("D:")) {
                first_file = "D:/YM-Code/LS-NTGF-Data-Cap/" + first_file;
            }
            log_widget_->AppendLog("Generated file ready for solving: " + first_file);
        }
    } else {
        statusBar()->showMessage("Generation failed");
        log_widget_->AppendLog("Generation failed: " + message);
    }
}
