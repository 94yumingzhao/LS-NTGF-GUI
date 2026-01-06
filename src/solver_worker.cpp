// solver_worker.cpp - Background Solver Worker (Subprocess) Implementation

#include "solver_worker.h"

#include <QCoreApplication>
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
    , solver_process_(nullptr)
    , log_reader_(nullptr)
    , log_file_pos_(0)
    , cancel_requested_(false) {
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
                                  int b_penalty, bool merge_enabled, double big_order_threshold) {
    runtime_limit_ = runtime_limit;
    u_penalty_ = u_penalty;
    b_penalty_ = b_penalty;
    merge_enabled_ = merge_enabled;
    big_order_threshold_ = big_order_threshold;
}

QString SolverWorker::GetAlgorithmName() const {
    switch (algorithm_) {
        case AlgorithmType::RF:  return "RF";
        case AlgorithmType::RFO: return "RFO";
        case AlgorithmType::RR:  return "RR";
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
        emit LogMessage(QString::fromUtf8("Error: Solver not found: %1").arg(exe_path));
        emit OptimizationFinished(false, QString::fromUtf8("Solver executable not found"));
        return;
    }

    emit LogMessage(QString::fromUtf8("Solver: %1").arg(exe_path));
    emit LogMessage(QString::fromUtf8("Algorithm: %1").arg(GetAlgorithmName()));
    emit LogMessage(QString::fromUtf8("Data: %1").arg(data_path_));

    // Prepare temporary directories
    QString temp_dir = QDir::tempPath() + "/LS-NTGF-GUI";
    QDir().mkpath(temp_dir);
    QDir().mkpath(temp_dir + "/results");
    QDir().mkpath(temp_dir + "/logs");

    log_file_path_ = temp_dir + "/logs/solve_" + GetAlgorithmName() + ".log";
    log_file_pos_ = 0;

    // Build command line arguments
    QStringList args;
    args << QString("--algo=%1").arg(GetAlgorithmName());
    args << "-f" << data_path_;
    args << "-o" << (temp_dir + "/results");
    args << "-l" << (temp_dir + "/logs/solve_" + GetAlgorithmName());
    args << "-t" << QString::number(runtime_limit_, 'f', 1);
    args << "--u-penalty" << QString::number(u_penalty_);
    args << "--b-penalty" << QString::number(b_penalty_);
    if (merge_enabled_) {
        args << "--threshold" << QString::number(big_order_threshold_, 'f', 1);
    } else {
        args << "--no-merge";
    }

    emit LogMessage(QString::fromUtf8("Args: %1").arg(args.join(" ")));

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
        emit LogMessage(QString::fromUtf8("Error: Failed to start solver process"));
        emit OptimizationFinished(false, QString::fromUtf8("Failed to start solver"));
        return;
    }

    emit LogMessage(QString::fromUtf8("Solver process started (PID: %1)")
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
        emit OptimizationFinished(false, QString::fromUtf8("Cancelled by user"));
        return;
    }

    if (status == QProcess::CrashExit) {
        emit LogMessage(QString::fromUtf8("Solver process crashed"));
        emit OptimizationFinished(false, QString::fromUtf8("Solver crashed"));
        return;
    }

    if (exitCode != 0) {
        emit LogMessage(QString::fromUtf8("Solver exited with code %1").arg(exitCode));
        emit OptimizationFinished(false, QString::fromUtf8("Solver failed (exit code %1)").arg(exitCode));
        return;
    }

    emit LogMessage(QString::fromUtf8("Solver completed successfully"));
    emit OptimizationFinished(true, QString::fromUtf8("Completed"));
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
            case 1: name = QString::fromUtf8("Stage 1 - Setup Optimization"); break;
            case 2: name = QString::fromUtf8("Stage 2 - Carryover Optimization"); break;
            case 3: name = QString::fromUtf8("Stage 3 - Final Optimization"); break;
            default: name = QString::fromUtf8("Stage %1").arg(stage); break;
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
        emit LogMessage(QString::fromUtf8("Error: %1").arg(message));
        return;
    }
}
