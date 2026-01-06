// generator_worker.cpp - Background Generator Worker Implementation

#include "generator_worker.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QRegularExpression>

GeneratorWorker::GeneratorWorker(QObject* parent)
    : QObject(parent)
    , generator_process_(nullptr)
    , current_instance_(0)
    , cancel_requested_(false) {
}

GeneratorWorker::~GeneratorWorker() {
    if (generator_process_) {
        generator_process_->kill();
        generator_process_->waitForFinished(1000);
        delete generator_process_;
    }
}

void GeneratorWorker::SetConfig(const GeneratorConfig& config) {
    config_ = config;
}

QString GeneratorWorker::GetGeneratorExePath() const {
    // Look for the generator executable relative to GUI location
    QString app_dir = QCoreApplication::applicationDirPath();

    // Try common locations
    QStringList candidates = {
        app_dir + "/OrderGenCap.exe",
        app_dir + "/../../../LS-NTGF-Data-Cap/build/vs2022-release/bin/Debug/OrderGenCap.exe",
        "D:/YM-Code/LS-NTGF-Data-Cap/build/vs2022-release/bin/Debug/OrderGenCap.exe",
        "D:/YM-Code/LS-NTGF-Data-Cap/build/vs2022-release/bin/Release/OrderGenCap.exe"
    };

    for (const QString& path : candidates) {
        if (QFile::exists(path)) {
            return path;
        }
    }

    return QString();
}

QString GeneratorWorker::BuildConfigFile() const {
    // Create a temporary config file for the generator
    QString config_path = config_.output_path + "/.gen_config.tmp";

    QFile file(config_path);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        out << "N=" << config_.N << "\n";
        out << "T=" << config_.T << "\n";
        out << "F=" << config_.F << "\n";
        out << "G=" << config_.G << "\n";
        out << "capacity_utilization=" << config_.capacity_utilization << "\n";
        out << "time_window_offset=" << config_.time_window_offset << "\n";
        out << "demand_cv=" << config_.demand_cv << "\n";
        out << "peak_ratio=" << config_.peak_ratio << "\n";
        out << "peak_multiplier=" << config_.peak_multiplier << "\n";
        out << "urgent_ratio=" << config_.urgent_ratio << "\n";
        out << "flexible_ratio=" << config_.flexible_ratio << "\n";
        out << "cost_correlation=" << (config_.cost_correlation ? 1 : 0) << "\n";
        out << "zoom=" << config_.zoom << "\n";
        out << "seed=" << config_.seed << "\n";
        out << "count=" << config_.count << "\n";
        file.close();
        return config_path;
    }

    return QString();
}

void GeneratorWorker::RunGeneration() {
    cancel_requested_ = false;
    generated_files_.clear();
    current_instance_ = 0;

    QString exe_path = GetGeneratorExePath();
    if (exe_path.isEmpty()) {
        emit GenerationFinished(false, "Generator executable not found", QStringList());
        return;
    }

    emit GenerationStarted(config_.count);
    emit LogMessage(QString("Generator: %1").arg(exe_path));
    emit LogMessage(QString("Output: %1").arg(config_.output_path));
    emit LogMessage(QString("Config: N=%1 T=%2 F=%3 G=%4 util=%5 count=%6")
        .arg(config_.N).arg(config_.T).arg(config_.F).arg(config_.G)
        .arg(config_.capacity_utilization, 0, 'f', 2).arg(config_.count));

    // The current generator doesn't support command-line config,
    // so we run it directly with hardcoded params.
    // For now, run the generator once (it generates based on its internal config)
    generator_process_ = new QProcess(this);

    connect(generator_process_, &QProcess::readyReadStandardOutput,
            this, &GeneratorWorker::OnProcessOutput);
    connect(generator_process_, &QProcess::readyReadStandardError,
            this, &GeneratorWorker::OnProcessOutput);
    connect(generator_process_, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &GeneratorWorker::OnProcessFinished);

    // Set working directory
    generator_process_->setWorkingDirectory("D:/YM-Code/LS-NTGF-Data-Cap");
    generator_process_->start(exe_path);

    if (!generator_process_->waitForStarted(5000)) {
        emit GenerationFinished(false, "Failed to start generator process", QStringList());
        delete generator_process_;
        generator_process_ = nullptr;
        return;
    }
}

void GeneratorWorker::RequestCancel() {
    cancel_requested_ = true;
    if (generator_process_ && generator_process_->state() == QProcess::Running) {
        generator_process_->kill();
    }
}

void GeneratorWorker::OnProcessOutput() {
    if (!generator_process_) return;

    QString output = generator_process_->readAllStandardOutput();
    output += generator_process_->readAllStandardError();

    // Remove ANSI color codes
    output.remove(QRegularExpression("\\x1B\\[[0-9;]*m"));

    QStringList lines = output.split('\n', Qt::SkipEmptyParts);
    for (const QString& line : lines) {
        emit LogMessage(line.trimmed());

        // Check for file output
        if (line.contains("data/") && line.contains(".csv")) {
            QRegularExpression rx("(data/[^\\s]+\\.csv)");
            QRegularExpressionMatch match = rx.match(line);
            if (match.hasMatch()) {
                QString filename = match.captured(1);
                generated_files_.append(filename);
                emit InstanceGenerated(generated_files_.size(), filename);
            }
        }
    }
}

void GeneratorWorker::OnProcessFinished(int exitCode, QProcess::ExitStatus status) {
    bool success = (exitCode == 0 && status == QProcess::NormalExit && !cancel_requested_);

    QString message;
    if (cancel_requested_) {
        message = "Generation cancelled";
    } else if (success) {
        message = QString("Generated %1 instance(s)").arg(generated_files_.size());
    } else {
        message = QString("Generation failed (exit code: %1)").arg(exitCode);
    }

    emit GenerationFinished(success, message, generated_files_);

    delete generator_process_;
    generator_process_ = nullptr;
}
