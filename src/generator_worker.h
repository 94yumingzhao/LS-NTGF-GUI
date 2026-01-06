// generator_worker.h - Background Generator Worker
//
// Runs the LS-NTGF-Data-Cap generator as a subprocess

#ifndef GENERATOR_WORKER_H_
#define GENERATOR_WORKER_H_

#include <QObject>
#include <QString>
#include <QProcess>
#include <atomic>
#include "difficulty_mapper.h"

class GeneratorWorker : public QObject {
    Q_OBJECT

public:
    explicit GeneratorWorker(QObject* parent = nullptr);
    ~GeneratorWorker() override;

    void SetConfig(const GeneratorConfig& config);

public slots:
    void RunGeneration();
    void RequestCancel();

signals:
    void GenerationStarted(int count);
    void InstanceGenerated(int index, const QString& filename);
    void GenerationFinished(bool success, const QString& message, const QStringList& files);
    void LogMessage(const QString& message);

private slots:
    void OnProcessOutput();
    void OnProcessFinished(int exitCode, QProcess::ExitStatus status);

private:
    QString GetGeneratorExePath() const;
    QString BuildConfigFile() const;

    GeneratorConfig config_;
    QProcess* generator_process_;
    QStringList generated_files_;
    int current_instance_;
    std::atomic<bool> cancel_requested_;
};

#endif  // GENERATOR_WORKER_H_
