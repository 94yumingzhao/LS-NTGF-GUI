// main_window.cpp - Main Window Implementation
// Layout: Left sidebar + Right log area

#include "main_window.h"
#include "parameter_widget.h"
#include "results_widget.h"
#include "log_widget.h"
#include "cplex_settings_widget.h"
#include "solver_worker.h"
#include "generator_widget.h"
#include "generator_worker.h"
#include "analysis_widget.h"

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
#include <QRegularExpression>
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
    auto* main_layout = new QVBoxLayout(central);
    main_layout->setSpacing(0);
    main_layout->setContentsMargins(0, 0, 0, 0);

    // 顶层Tab: 求解 | 算例生成 | 结果分析
    mode_tabs_ = new QTabWidget(this);

    // ===== Tab 1: 求解页 =====
    auto* solver_page = new QWidget(this);
    auto* solver_page_layout = new QHBoxLayout(solver_page);
    solver_page_layout->setSpacing(8);
    solver_page_layout->setContentsMargins(8, 8, 8, 8);

    // 左侧控制面板
    auto* solver_left = new QWidget(this);
    solver_left->setMinimumWidth(260);
    solver_left->setMaximumWidth(400);
    auto* solver_left_layout = new QVBoxLayout(solver_left);
    solver_left_layout->setSpacing(8);
    solver_left_layout->setContentsMargins(0, 0, 0, 0);

    // 文件选择
    file_group_ = new QGroupBox(QString::fromUtf8("数据文件"), solver_left);
    auto* file_layout = new QVBoxLayout(file_group_);
    file_layout->setSpacing(4);
    file_layout->setContentsMargins(8, 12, 8, 8);

    auto* browse_layout = new QHBoxLayout();
    browse_button_ = new QPushButton(QString::fromUtf8("..."), this);
    browse_button_->setFixedWidth(32);
    file_path_edit_ = new QLineEdit(this);
    file_path_edit_->setReadOnly(true);
    file_path_edit_->setPlaceholderText(QString::fromUtf8("选择CSV文件..."));
    browse_layout->addWidget(file_path_edit_);
    browse_layout->addWidget(browse_button_);
    file_layout->addLayout(browse_layout);

    file_info_label_ = new QLabel(QString::fromUtf8("--"), this);
    file_info_label_->setStyleSheet("color: #666; font-size: 9pt;");
    file_layout->addWidget(file_info_label_);

    solver_left_layout->addWidget(file_group_);

    // 算法参数
    param_widget_ = new ParameterWidget(solver_left);
    solver_left_layout->addWidget(param_widget_);

    // 运行控制
    control_group_ = new QGroupBox(QString::fromUtf8("运行"), solver_left);
    auto* control_layout = new QVBoxLayout(control_group_);
    control_layout->setSpacing(6);
    control_layout->setContentsMargins(8, 12, 8, 8);

    auto* button_layout = new QHBoxLayout();
    start_button_ = new QPushButton(QString::fromUtf8("运行"), this);
    start_button_->setMinimumHeight(32);
    start_button_->setStyleSheet("font-weight: bold;");
    cancel_button_ = new QPushButton(QString::fromUtf8("取消"), this);
    cancel_button_->setMinimumHeight(32);
    button_layout->addWidget(start_button_);
    button_layout->addWidget(cancel_button_);
    control_layout->addLayout(button_layout);

    status_label_ = new QLabel(QString::fromUtf8("就绪"), this);
    status_label_->setAlignment(Qt::AlignCenter);
    status_label_->setStyleSheet("color: #666;");
    control_layout->addWidget(status_label_);

    solver_left_layout->addWidget(control_group_);

    // 结果摘要
    results_widget_ = new ResultsWidget(solver_left);
    solver_left_layout->addWidget(results_widget_);

    // 导出按钮
    export_button_ = new QPushButton(QString::fromUtf8("导出日志..."), solver_left);
    solver_left_layout->addWidget(export_button_);

    solver_left_layout->addStretch();

    // 右侧面板 (CPLEX设置 + 日志)
    auto* solver_right = new QWidget(this);
    auto* solver_right_layout = new QVBoxLayout(solver_right);
    solver_right_layout->setSpacing(8);
    solver_right_layout->setContentsMargins(0, 0, 0, 0);

    cplex_settings_widget_ = new CplexSettingsWidget(solver_right);
    solver_right_layout->addWidget(cplex_settings_widget_);

    log_widget_ = new LogWidget(solver_right);
    solver_right_layout->addWidget(log_widget_, 1);

    // Splitter with visible handle
    main_splitter_ = new QSplitter(Qt::Horizontal, this);
    main_splitter_->addWidget(solver_left);
    main_splitter_->addWidget(solver_right);
    main_splitter_->setStretchFactor(0, 0);
    main_splitter_->setStretchFactor(1, 1);
    main_splitter_->setHandleWidth(6);
    main_splitter_->setStyleSheet(
        "QSplitter::handle { background-color: #dee2e6; }"
        "QSplitter::handle:hover { background-color: #adb5bd; }"
        "QSplitter::handle:pressed { background-color: #868e96; }");

    solver_page_layout->addWidget(main_splitter_);

    // ===== Tab 2: 算例生成页 =====
    auto* generator_page = new QWidget(this);
    auto* generator_page_layout = new QHBoxLayout(generator_page);
    generator_page_layout->setSpacing(8);
    generator_page_layout->setContentsMargins(8, 8, 8, 8);

    // 左侧生成器控制
    generator_widget_ = new GeneratorWidget();
    generator_widget_->setMinimumWidth(300);
    generator_widget_->setMaximumWidth(500);

    // 右侧日志
    generator_log_widget_ = new LogWidget(generator_page);

    // Splitter with visible handle
    auto* generator_splitter = new QSplitter(Qt::Horizontal, generator_page);
    generator_splitter->addWidget(generator_widget_);
    generator_splitter->addWidget(generator_log_widget_);
    generator_splitter->setStretchFactor(0, 0);
    generator_splitter->setStretchFactor(1, 1);
    generator_splitter->setHandleWidth(6);
    generator_splitter->setStyleSheet(
        "QSplitter::handle { background-color: #dee2e6; }"
        "QSplitter::handle:hover { background-color: #adb5bd; }"
        "QSplitter::handle:pressed { background-color: #868e96; }");

    generator_page_layout->addWidget(generator_splitter);

    // ===== Tab 3: 结果分析页 (全宽) =====
    analysis_widget_ = new AnalysisWidget();

    // 添加三个顶层Tab
    mode_tabs_->addTab(solver_page, QString::fromUtf8("求解"));
    mode_tabs_->addTab(generator_page, QString::fromUtf8("算例生成"));
    mode_tabs_->addTab(analysis_widget_, QString::fromUtf8("结果分析"));

    main_layout->addWidget(mode_tabs_);
    setCentralWidget(central);

    statusBar()->showMessage(QString::fromUtf8("就绪"));
}

void MainWindow::SetupMenuBar() {
    auto* file_menu = menuBar()->addMenu(QString::fromUtf8("\u6587\u4ef6(&F)"));

    auto* open_action = new QAction(QString::fromUtf8("\u6253\u5f00(&O)..."), this);
    open_action->setShortcut(QKeySequence::Open);
    connect(open_action, &QAction::triggered, this, &MainWindow::OnBrowseFile);
    file_menu->addAction(open_action);

    auto* export_action = new QAction(QString::fromUtf8("\u5bfc\u51fa\u65e5\u5fd7(&E)..."), this);
    export_action->setShortcut(QKeySequence::Save);
    connect(export_action, &QAction::triggered, this, &MainWindow::OnExportLog);
    file_menu->addAction(export_action);

    file_menu->addSeparator();

    auto* exit_action = new QAction(QString::fromUtf8("\u9000\u51fa(&X)"), this);
    exit_action->setShortcut(QKeySequence::Quit);
    connect(exit_action, &QAction::triggered, this, &QWidget::close);
    file_menu->addAction(exit_action);

    auto* help_menu = menuBar()->addMenu(QString::fromUtf8("\u5e2e\u52a9(&H)"));
    auto* about_action = new QAction(QString::fromUtf8("\u5173\u4e8e(&A)"), this);
    connect(about_action, &QAction::triggered, [this]() {
        QMessageBox::about(this, QString::fromUtf8("\u5173\u4e8e"),
            QString::fromUtf8(
                "LS-NTGF GUI\n\n"
                "\u7248\u672c 2.0.0\n\n"
                "\u7b97\u6cd5:\n"
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
            this, &MainWindow::OnGeneratorLogMessage);

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
        QString::fromUtf8("\u8fd0\u884c\u4e2d...") :
        QString::fromUtf8("\u5c31\u7eea"));
}

void MainWindow::ResetState() {
    results_widget_->ClearResults();
    log_widget_->ClearLog();
    total_runtime_ = 0.0;
}

void MainWindow::OnBrowseFile() {
    QString path = QFileDialog::getOpenFileName(this,
        QString::fromUtf8("打开数据文件"),
        "D:/YM-Code/LS-NTGF-Data-Cap/data",
        QString::fromUtf8("CSV (*.csv);;所有文件 (*)"));

    if (!path.isEmpty()) {
        current_file_path_ = path;
        // Show only filename in the edit
        QFileInfo fi(path);
        file_path_edit_->setText(fi.fileName());
        file_path_edit_->setToolTip(path);

        // Parse CSV to get NTGF indicators
        ParseCsvForIndicators(path);

        start_button_->setEnabled(true);
        log_widget_->AppendLog(QString::fromUtf8("文件: ") + path);
    }
}

void MainWindow::ParseCsvForIndicators(const QString& path) {
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        file_info_label_->setText(QString::fromUtf8("无法读取文件"));
        file_info_label_->setStyleSheet("color: red; font-size: 9pt;");
        return;
    }

    int n = 0, t = 0, g = 0, f = 0;
    QTextStream in(&file);
    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (line.startsWith("T_num,")) {
            t = line.mid(6).toInt();
        } else if (line.startsWith("F_num,")) {
            f = line.mid(6).toInt();
        } else if (line.startsWith("G_num,")) {
            g = line.mid(6).toInt();
        } else if (line.startsWith("Order_Num,")) {
            n = line.mid(10).toInt();
        }
        // Stop after finding all values
        if (n > 0 && t > 0 && g > 0 && f > 0) break;
    }
    file.close();

    if (n > 0 && t > 0) {
        inst_n_ = n;
        inst_t_ = t;
        inst_g_ = g;
        inst_f_ = f;

        // 尝试从文件名提取难度 (格式: N100T30G5F5_0.93_20260106_...)
        QFileInfo fi(path);
        QRegularExpression re("_(\\d+\\.\\d+)_\\d{8}_");
        QRegularExpressionMatch match = re.match(fi.fileName());
        if (match.hasMatch()) {
            inst_difficulty_ = match.captured(1).toDouble();
        } else {
            // 使用简化公式计算难度 (默认参数)
            inst_difficulty_ = 0.30 * 1.0
                + 0.20 * (1.0 - 11.0 / t)
                + 0.20 * (static_cast<double>(n) * t / 3000.0)
                + 0.15 * 1.0
                + 0.15 * (static_cast<double>(g) / 5.0);
        }

        QString info = QString("N=%1  T=%2  G=%3  F=%4").arg(n).arg(t).arg(g).arg(f);
        file_info_label_->setText(info);
        file_info_label_->setStyleSheet("color: black; font-size: 9pt;");
    } else {
        inst_n_ = inst_t_ = inst_g_ = inst_f_ = 0;
        inst_difficulty_ = 0.0;
        file_info_label_->setText(QString::fromUtf8("无法解析文件"));
        file_info_label_->setStyleSheet("color: orange; font-size: 9pt;");
    }
}

void MainWindow::OnStartOptimization() {
    if (current_file_path_.isEmpty()) {
        QMessageBox::warning(this, QString::fromUtf8("\u9519\u8bef"),
            QString::fromUtf8("\u8bf7\u5148\u9009\u62e9\u6570\u636e\u6587\u4ef6"));
        return;
    }

    ResetState();
    UpdateUiState(true);

    // 设置算法和参数
    int algo_idx = param_widget_->GetAlgorithmIndex();
    AlgorithmType algo = static_cast<AlgorithmType>(algo_idx);
    solver_worker_->SetAlgorithm(algo);
    solver_worker_->SetDataPath(current_file_path_);
    solver_worker_->SetParameters(
        param_widget_->GetRuntimeLimit(),
        param_widget_->GetUPenalty(),
        param_widget_->GetBPenalty(),
        param_widget_->GetMergeEnabled(),
        param_widget_->GetBigOrderThreshold(),
        param_widget_->GetMachineCapacity()
    );
    solver_worker_->SetCplexParameters(
        cplex_settings_widget_->GetWorkDir(),
        cplex_settings_widget_->GetWorkMem(),
        cplex_settings_widget_->GetThreads()
    );
    // Set advanced algorithm parameters
    solver_worker_->SetRFParameters(
        param_widget_->GetRFWindow(),
        param_widget_->GetRFStep(),
        param_widget_->GetRFTime(),
        param_widget_->GetRFRetries()
    );
    solver_worker_->SetFOParameters(
        param_widget_->GetFOWindow(),
        param_widget_->GetFOStep(),
        param_widget_->GetFORounds(),
        param_widget_->GetFOBuffer(),
        param_widget_->GetFOTime()
    );
    solver_worker_->SetRRParameters(
        param_widget_->GetRRCapacity(),
        param_widget_->GetRRBonus()
    );
    solver_worker_->SetInstanceInfo(inst_n_, inst_t_, inst_g_, inst_f_, inst_difficulty_);

    QString algo_names[] = {"RF", "RFO", "RR"};
    log_widget_->AppendLog(QString::fromUtf8("开始优化 (算法: %1)...")
        .arg(algo_names[algo_idx]));
    statusBar()->showMessage(QString::fromUtf8("优化中..."));

    log_widget_->StartTimer();
    emit StartSolver();
}

void MainWindow::OnCancelOptimization() {
    if (solver_worker_) {
        solver_worker_->RequestCancel();
        log_widget_->AppendLog(QString::fromUtf8("\u53d6\u6d88\u4e2d..."));
    }
}

void MainWindow::OnExportLog() {
    QString path = QFileDialog::getSaveFileName(this,
        QString::fromUtf8("\u5bfc\u51fa\u65e5\u5fd7"),
        "log.txt",
        QString::fromUtf8("\u6587\u672c\u6587\u4ef6 (*.txt);;\u6240\u6709\u6587\u4ef6 (*)"));

    if (!path.isEmpty()) {
        QFile file(path);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream out(&file);
            out << log_widget_->GetLogText();
            file.close();
            log_widget_->AppendLog(QString::fromUtf8("\u65e5\u5fd7\u5df2\u5bfc\u51fa: ") + path);
            QMessageBox::information(this, QString::fromUtf8("\u5bfc\u51fa"),
                QString::fromUtf8("\u65e5\u5fd7\u5bfc\u51fa\u6210\u529f"));
        } else {
            QMessageBox::warning(this, QString::fromUtf8("\u5bfc\u51fa\u9519\u8bef"),
                QString::fromUtf8("\u65e5\u5fd7\u5bfc\u51fa\u5931\u8d25"));
        }
    }
}

void MainWindow::OnAlgorithmChanged(int index) {
    AlgorithmType algo = static_cast<AlgorithmType>(index);
    results_widget_->SetAlgorithmType(algo);
}

void MainWindow::OnDataLoaded(int items, int periods, int flows, int groups) {
    QString info = QString::fromUtf8("\u8ba2\u5355:%1 \u5468\u671f:%2 \u6d41\u5411:%3 \u7ec4\u522b:%4")
        .arg(items).arg(periods).arg(flows).arg(groups);
    file_info_label_->setText(info);
    file_info_label_->setStyleSheet("color: black; font-size: 9pt;");
    log_widget_->AppendLog(QString::fromUtf8("\u6570\u636e\u5df2\u52a0\u8f7d: %1 \u8ba2\u5355, %2 \u5468\u671f")
        .arg(items).arg(periods));
}

void MainWindow::OnOrdersMerged(int original, int merged) {
    log_widget_->AppendLog(QString::fromUtf8("\u8ba2\u5355\u5408\u5e76: %1 -> %2")
        .arg(original).arg(merged));
    results_widget_->SetMergeInfo(original, merged);
}

void MainWindow::OnMergeSkipped() {
    log_widget_->AppendLog(QString::fromUtf8("\u8ba2\u5355\u5408\u5e76: \u5df2\u8df3\u8fc7"));
    results_widget_->SetMergeSkipped();
}

void MainWindow::OnStageStarted(int stage, const QString& name) {
    log_widget_->AppendLog(QString::fromUtf8("\u5f00\u59cb: ") + name);
}

void MainWindow::OnStageCompleted(int stage, double objective, double runtime, double gap) {
    total_runtime_ += runtime;

    // Update results widget
    results_widget_->SetStageResult(stage, objective, runtime, gap);

    log_widget_->AppendLog(QString::fromUtf8("\u9636\u6bb5%1\u5b8c\u6210: \u76ee\u6807=%2, \u8017\u65f6=%3s, Gap=%4%")
        .arg(stage).arg(objective, 0, 'f', 2).arg(runtime, 0, 'f', 3).arg(gap * 100, 0, 'f', 4));
}

void MainWindow::OnOptimizationFinished(bool success, const QString& message) {
    log_widget_->StopTimer();
    UpdateUiState(false);

    results_widget_->SetTotalRuntime(total_runtime_);

    if (success) {
        statusBar()->showMessage(QString::fromUtf8("优化完成"));
        log_widget_->AppendLog(QString::fromUtf8("完成: ") + message);
    } else {
        statusBar()->showMessage(QString::fromUtf8("优化已停止"));
        log_widget_->AppendLog(QString::fromUtf8("已停止: ") + message);
        QMessageBox::warning(this, QString::fromUtf8("优化"), message);
    }
}

void MainWindow::OnLogMessage(const QString& message) {
    log_widget_->AppendLog(message);
}

// ============================================================================
// Generator Slots
// ============================================================================

void MainWindow::OnGenerateRequested(const GeneratorConfig& config) {
    generator_log_widget_->ClearLog();
    generator_log_widget_->AppendLog(QString::fromUtf8("开始生成算例..."));
    generator_log_widget_->StartTimer();

    generator_worker_->SetConfig(config);

    // Use QMetaObject to invoke across threads
    QMetaObject::invokeMethod(generator_worker_, "RunGeneration", Qt::QueuedConnection);

    statusBar()->showMessage(QString::fromUtf8("生成中..."));
}

void MainWindow::OnGenerationStarted(int count) {
    generator_log_widget_->AppendLog(QString::fromUtf8("正在生成 %1 个算例...").arg(count));
}

void MainWindow::OnInstanceGenerated(int index, const QString& filename) {
    generator_log_widget_->AppendLog(QString::fromUtf8("[%1] 已生成: %2").arg(index).arg(filename));
}

void MainWindow::OnGenerationFinished(bool success, const QString& message,
                                       const QStringList& files) {
    generator_log_widget_->StopTimer();

    if (success) {
        statusBar()->showMessage(QString::fromUtf8("生成完成"));
        generator_log_widget_->AppendLog(QString::fromUtf8("生成完成: ") + message);

        // Offer to load the first generated file
        if (!files.isEmpty()) {
            QString first_file = files.first();
            if (!first_file.startsWith("D:")) {
                first_file = "D:/YM-Code/LS-NTGF-Data-Cap/" + first_file;
            }
            generator_log_widget_->AppendLog(QString::fromUtf8("算例已生成，可用于求解: ") + first_file);
        }
    } else {
        statusBar()->showMessage(QString::fromUtf8("生成失败"));
        generator_log_widget_->AppendLog(QString::fromUtf8("生成失败: ") + message);
    }
}

void MainWindow::OnGeneratorLogMessage(const QString& message) {
    generator_log_widget_->AppendLog(message);
}
