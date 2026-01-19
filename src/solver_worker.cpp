// solver_worker.cpp - Background Solver Worker (Subprocess) Implementation

#include "solver_worker.h"

#include <QCoreApplication>
#include <QDateTime>
#include <QDir>
#include <QFileInfo>
#include <QRegularExpression>
#include <QTextStream>

SolverWorker::SolverWorker(QObject* parent)
    : QObject(parent)
    , algorithm_(AlgorithmType::RF)
    , runtime_limit_(30.0)
    , u_penalty_(10000)
    , b_penalty_(100)
    , merge_enabled_(true)
    , big_order_threshold_(1000.0)
    , machine_capacity_(1440)
    , solver_process_(nullptr)
    , log_reader_(nullptr)
    , log_file_pos_(0)
    , cancel_requested_(false)
    // RF defaults
    , rf_window_(6)
    , rf_step_(1)
    , rf_time_(60.0)
    , rf_retries_(3)
    // FO defaults
    , fo_window_(8)
    , fo_step_(3)
    , fo_rounds_(2)
    , fo_buffer_(1)
    , fo_time_(30.0)
    // RR defaults
    , rr_capacity_(1.2)
    , rr_bonus_(50.0)
    // LR defaults
    , lr_max_iter_(200)
    , lr_alpha0_(2.0)
    , lr_decay_(0.98)
    , lr_tol_(0.01) {
}

SolverWorker::~SolverWorker() {
    if (solver_process_) {
        solver_process_->kill();
        solver_process_->waitForFinished(1000);
        delete solver_process_;
    }
    delete log_reader_;
}

void SolverWorker::SetDataPath(const QString& path) {
    data_path_ = path;
}

void SolverWorker::SetAlgorithm(AlgorithmType algo) {
    algorithm_ = algo;
}

void SolverWorker::SetParameters(double runtime_limit, int u_penalty,
                                  int b_penalty, bool merge_enabled, double big_order_threshold,
                                  int machine_capacity) {
    runtime_limit_ = runtime_limit;
    u_penalty_ = u_penalty;
    b_penalty_ = b_penalty;
    merge_enabled_ = merge_enabled;
    big_order_threshold_ = big_order_threshold;
    machine_capacity_ = machine_capacity;
}

void SolverWorker::SetRFParameters(int window, int step, double time, int retries) {
    rf_window_ = window;
    rf_step_ = step;
    rf_time_ = time;
    rf_retries_ = retries;
}

void SolverWorker::SetFOParameters(int window, int step, int rounds, int buffer, double time) {
    fo_window_ = window;
    fo_step_ = step;
    fo_rounds_ = rounds;
    fo_buffer_ = buffer;
    fo_time_ = time;
}

void SolverWorker::SetRRParameters(double capacity, double bonus) {
    rr_capacity_ = capacity;
    rr_bonus_ = bonus;
}

void SolverWorker::SetLRParameters(int max_iter, double alpha0, double decay, double tol) {
    lr_max_iter_ = max_iter;
    lr_alpha0_ = alpha0;
    lr_decay_ = decay;
    lr_tol_ = tol;
}

void SolverWorker::SetCplexParameters(const QString& workdir, int workmem, int threads) {
    cplex_workdir_ = workdir;
    cplex_workmem_ = workmem;
    cplex_threads_ = threads;
}

void SolverWorker::SetInstanceInfo(int n, int t, int g, int f, double difficulty) {
    inst_n_ = n;
    inst_t_ = t;
    inst_g_ = g;
    inst_f_ = f;
    inst_difficulty_ = difficulty;
}

QString SolverWorker::GetAlgorithmName() const {
    switch (algorithm_) {
        case AlgorithmType::RF:  return "RF";
        case AlgorithmType::RFO: return "RFO";
        case AlgorithmType::RR:  return "RR";
        case AlgorithmType::LR:  return "LR";
        default: return "RF";
    }
}

QString SolverWorker::GetSolverExePath() const {
    // Get the GUI executable directory
    QString app_dir = QCoreApplication::applicationDirPath();

    // Try relative path from GUI build directory
    // GUI: D:/YM-Code/LS-NTGF-GUI/build/vs2022/bin/Release/
    // Solver: D:/YM-Code/LS-NTGF-All/build/release/bin/Release/
    QStringList possible_paths = {
        app_dir + "/../../../../LS-NTGF-All/build/release/bin/Release/LS-NTGF-All.exe",
        app_dir + "/../../../LS-NTGF-All/build/release/bin/Release/LS-NTGF-All.exe",
        app_dir + "/../../LS-NTGF-All/build/release/bin/Release/LS-NTGF-All.exe",
        "D:/YM-Code/LS-NTGF-All/build/release/bin/Release/LS-NTGF-All.exe"
    };

    for (const QString& path : possible_paths) {
        QFileInfo fi(path);
        if (fi.exists()) {
            return fi.absoluteFilePath();
        }
    }

    // Fallback to hardcoded path
    return "D:/YM-Code/LS-NTGF-All/build/release/bin/Release/LS-NTGF-All.exe";
}

void SolverWorker::RequestCancel() {
    cancel_requested_ = true;
    if (solver_process_ && solver_process_->state() != QProcess::NotRunning) {
        solver_process_->kill();
    }
}

void SolverWorker::RunOptimization() {
    cancel_requested_ = false;

    // Get solver executable path
    QString exe_path = GetSolverExePath();
    QFileInfo exe_info(exe_path);

    if (!exe_info.exists()) {
        emit LogMessage(QString::fromUtf8("错误: 找不到求解器: %1").arg(exe_path));
        emit OptimizationFinished(false, QString::fromUtf8("找不到求解器可执行文件"));
        return;
    }

    emit LogMessage(QString::fromUtf8("求解器: %1").arg(exe_path));
    emit LogMessage(QString::fromUtf8("算法: %1").arg(GetAlgorithmName()));
    emit LogMessage(QString::fromUtf8("数据: %1").arg(data_path_));

    // 准备输出目录 (使用求解器自身的目录)
    QString logs_dir = "D:/YM-Code/LS-NTGF-All/logs";
    QString results_dir = "D:/YM-Code/LS-NTGF-All/results";
    QDir().mkpath(logs_dir);
    QDir().mkpath(results_dir);

    // 生成时间戳和文件名
    QString timestamp = QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss");
    QString file_base = QString("N%1T%2G%3F%4_%5_%6")
        .arg(inst_n_).arg(inst_t_).arg(inst_g_).arg(inst_f_)
        .arg(inst_difficulty_, 0, 'f', 2)
        .arg(timestamp);

    QString log_base = logs_dir + "/log_" + GetAlgorithmName() + "_" + file_base;

    log_file_path_ = log_base + ".log";
    log_file_pos_ = 0;

    // 构建命令行参数
    QStringList args;
    args << QString("--algo=%1").arg(GetAlgorithmName());
    args << "-f" << data_path_;
    args << "-o" << results_dir;
    args << "-l" << log_base;
    args << "-t" << QString::number(runtime_limit_, 'f', 1);
    args << "--u-penalty" << QString::number(u_penalty_);
    args << "--b-penalty" << QString::number(b_penalty_);
    args << "--capacity" << QString::number(machine_capacity_);
    if (merge_enabled_) {
        args << "--threshold" << QString::number(big_order_threshold_, 'f', 1);
    } else {
        args << "--no-merge";
    }

    // CPLEX parameters
    if (!cplex_workdir_.isEmpty()) {
        args << "--cplex-workdir" << cplex_workdir_;
    }
    args << "--cplex-workmem" << QString::number(cplex_workmem_);
    args << "--cplex-threads" << QString::number(cplex_threads_);

    // RF parameters (for RF and RFO algorithms)
    if (algorithm_ == AlgorithmType::RF || algorithm_ == AlgorithmType::RFO) {
        args << "--rf-window" << QString::number(rf_window_);
        args << "--rf-step" << QString::number(rf_step_);
        args << "--rf-time" << QString::number(rf_time_, 'f', 1);
        args << "--rf-retries" << QString::number(rf_retries_);
    }

    // FO parameters (for RFO algorithm only)
    if (algorithm_ == AlgorithmType::RFO) {
        args << "--fo-window" << QString::number(fo_window_);
        args << "--fo-step" << QString::number(fo_step_);
        args << "--fo-rounds" << QString::number(fo_rounds_);
        args << "--fo-buffer" << QString::number(fo_buffer_);
        args << "--fo-time" << QString::number(fo_time_, 'f', 1);
    }

    // RR parameters (for RR algorithm only)
    if (algorithm_ == AlgorithmType::RR) {
        args << "--rr-capacity" << QString::number(rr_capacity_, 'f', 2);
        args << "--rr-bonus" << QString::number(rr_bonus_, 'f', 1);
    }

    // LR parameters (for LR algorithm only)
    if (algorithm_ == AlgorithmType::LR) {
        args << "--lr-maxiter" << QString::number(lr_max_iter_);
        args << "--lr-alpha0" << QString::number(lr_alpha0_, 'f', 2);
        args << "--lr-decay" << QString::number(lr_decay_, 'f', 3);
        args << "--lr-tol" << QString::number(lr_tol_, 'f', 4);
    }

    emit LogMessage(QString::fromUtf8("参数: %1").arg(args.join(" ")));

    // Create and configure process
    if (solver_process_) {
        delete solver_process_;
    }
    solver_process_ = new QProcess(this);
    solver_process_->setWorkingDirectory(exe_info.absolutePath());

    connect(solver_process_, &QProcess::readyReadStandardOutput,
            this, &SolverWorker::OnProcessOutput);
    connect(solver_process_, &QProcess::readyReadStandardError,
            this, &SolverWorker::OnProcessError);
    connect(solver_process_, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &SolverWorker::OnProcessFinished);

    // Start log file reader
    if (log_reader_) {
        log_reader_->stop();
        delete log_reader_;
    }
    log_reader_ = new QTimer(this);
    connect(log_reader_, &QTimer::timeout, this, &SolverWorker::OnReadLogFile);
    log_reader_->start(500);  // Read log every 500ms

    // Start the solver process
    solver_process_->start(exe_path, args);

    if (!solver_process_->waitForStarted(5000)) {
        emit LogMessage(QString::fromUtf8("错误: 无法启动求解器进程"));
        emit OptimizationFinished(false, QString::fromUtf8("无法启动求解器"));
        return;
    }

    emit LogMessage(QString::fromUtf8("求解器进程已启动 (PID: %1)")
                    .arg(solver_process_->processId()));
}

void SolverWorker::OnProcessOutput() {
    if (!solver_process_) return;

    QByteArray data = solver_process_->readAllStandardOutput();
    QString output = QString::fromUtf8(data);

    // Parse each line for status codes
    QStringList lines = output.split('\n', Qt::SkipEmptyParts);
    for (const QString& line : lines) {
        QString trimmed = line.trimmed();
        if (trimmed.startsWith('[') && trimmed.endsWith(']')) {
            ParseStatusLine(trimmed);
        }
    }
}

void SolverWorker::OnProcessError() {
    if (!solver_process_) return;

    QByteArray data = solver_process_->readAllStandardError();
    QString error = QString::fromUtf8(data);

    // Log stderr output
    QStringList lines = error.split('\n', Qt::SkipEmptyParts);
    for (const QString& line : lines) {
        emit LogMessage(QString::fromUtf8("[stderr] %1").arg(line.trimmed()));
    }
}

void SolverWorker::OnProcessFinished(int exitCode, QProcess::ExitStatus status) {
    // Stop log reader
    if (log_reader_) {
        log_reader_->stop();
    }

    // Read any remaining log content
    OnReadLogFile();

    if (cancel_requested_) {
        emit OptimizationFinished(false, QString::fromUtf8("已被用户取消"));
        return;
    }

    if (status == QProcess::CrashExit) {
        emit LogMessage(QString::fromUtf8("求解器进程崩溃"));
        emit OptimizationFinished(false, QString::fromUtf8("求解器崩溃"));
        return;
    }

    if (exitCode != 0) {
        emit LogMessage(QString::fromUtf8("求解器退出, 代码 %1").arg(exitCode));
        emit OptimizationFinished(false, QString::fromUtf8("求解器失败 (退出代码 %1)").arg(exitCode));
        return;
    }

    emit LogMessage(QString::fromUtf8("求解器成功完成"));
    emit OptimizationFinished(true, QString::fromUtf8("完成"));
}

void SolverWorker::OnReadLogFile() {
    QFile file(log_file_path_);
    if (!file.exists()) return;

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return;

    // Seek to last read position
    file.seek(log_file_pos_);

    QTextStream in(&file);
    in.setEncoding(QStringConverter::Utf8);

    while (!in.atEnd()) {
        QString line = in.readLine();
        emit LogMessage(line);
    }

    log_file_pos_ = file.pos();
    file.close();
}

void SolverWorker::ParseStatusLine(const QString& line) {
    // Parse status codes from solver stdout
    // Format examples:
    // [LOAD:OK:150:30:5:3]
    // [MERGE:150:120]
    // [STAGE:1:START]
    // [STAGE:1:DONE:12345.6:5.2:0.01]
    // [DONE:SUCCESS]
    // [ERROR:message]

    static QRegularExpression re_load(R"(\[LOAD:OK:(\d+):(\d+):(\d+):(\d+)\])");
    static QRegularExpression re_merge(R"(\[MERGE:(\d+):(\d+)\])");
    static QRegularExpression re_stage_start(R"(\[STAGE:(\d+):START\])");
    static QRegularExpression re_stage_done(R"(\[STAGE:(\d+):DONE:([^:]+):([^:]+):([^\]]+)\])");
    static QRegularExpression re_done(R"(\[DONE:SUCCESS\])");
    static QRegularExpression re_error(R"(\[ERROR:([^\]]+)\])");

    QRegularExpressionMatch match;

    // Check LOAD
    match = re_load.match(line);
    if (match.hasMatch()) {
        int items = match.captured(1).toInt();
        int periods = match.captured(2).toInt();
        int flows = match.captured(3).toInt();
        int groups = match.captured(4).toInt();
        emit DataLoaded(items, periods, flows, groups);
        return;
    }

    // Check MERGE
    match = re_merge.match(line);
    if (match.hasMatch()) {
        int original = match.captured(1).toInt();
        int merged = match.captured(2).toInt();
        emit OrdersMerged(original, merged);
        return;
    }

    // Check MERGE:SKIP
    if (line == "[MERGE:SKIP]") {
        emit MergeSkipped();
        return;
    }

    // Check STAGE START
    match = re_stage_start.match(line);
    if (match.hasMatch()) {
        int stage = match.captured(1).toInt();
        QString name;
        switch (stage) {
            case 1: name = QString::fromUtf8("阶段1 - 初始优化"); break;
            case 2: name = QString::fromUtf8("阶段2 - 跨期优化"); break;
            case 3: name = QString::fromUtf8("阶段3 - 最终优化"); break;
            default: name = QString::fromUtf8("阶段 %1").arg(stage); break;
        }
        emit StageStarted(stage, name);
        return;
    }

    // Check STAGE DONE
    match = re_stage_done.match(line);
    if (match.hasMatch()) {
        int stage = match.captured(1).toInt();
        double objective = match.captured(2).toDouble();
        double runtime = match.captured(3).toDouble();
        double gap = match.captured(4).toDouble();
        emit StageCompleted(stage, objective, runtime, gap);
        return;
    }

    // Check ERROR
    match = re_error.match(line);
    if (match.hasMatch()) {
        QString message = match.captured(1);
        emit LogMessage(QString::fromUtf8("错误: %1").arg(message));
        return;
    }
}
