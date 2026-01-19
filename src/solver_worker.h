// solver_worker.h - Background Solver Worker (Subprocess)
//
// 调用 LS-NTGF-All 求解器执行优化

#ifndef SOLVER_WORKER_H_
#define SOLVER_WORKER_H_

#include <QObject>
#include <QString>
#include <QProcess>
#include <QTimer>
#include <QFile>
#include <atomic>

// 算法类型
enum class AlgorithmType {
    RF,   // Relax-and-Fix
    RFO,  // RF + Fix-and-Optimize
    RR,   // PP-GCB 三阶段分解
    LR    // Lagrangian Relaxation
};

class SolverWorker : public QObject {
    Q_OBJECT

public:
    explicit SolverWorker(QObject* parent = nullptr);
    ~SolverWorker() override;

    void SetDataPath(const QString& path);
    void SetAlgorithm(AlgorithmType algo);
    void SetParameters(double runtime_limit, int u_penalty,
                       int b_penalty, bool merge_enabled, double big_order_threshold,
                       int machine_capacity);
    void SetCplexParameters(const QString& workdir, int workmem, int threads);
    void SetInstanceInfo(int n, int t, int g, int f, double difficulty);

    // Advanced algorithm parameters
    void SetRFParameters(int window, int step, double time, int retries);
    void SetFOParameters(int window, int step, int rounds, int buffer, double time);
    void SetRRParameters(double capacity, double bonus);
    void SetLRParameters(int max_iter, double alpha0, double decay, double tol);

    AlgorithmType GetAlgorithm() const { return algorithm_; }

public slots:
    void RunOptimization();
    void RequestCancel();

signals:
    void DataLoaded(int items, int periods, int flows, int groups);
    void OrdersMerged(int original, int merged);
    void MergeSkipped();
    void StageStarted(int stage, const QString& name);
    void StageCompleted(int stage, double objective, double runtime, double gap);
    void OptimizationFinished(bool success, const QString& message);
    void LogMessage(const QString& message);

private slots:
    void OnProcessOutput();
    void OnProcessError();
    void OnProcessFinished(int exitCode, QProcess::ExitStatus status);
    void OnReadLogFile();

private:
    void ParseStatusLine(const QString& line);
    QString GetSolverExePath() const;
    QString GetAlgorithmName() const;

    QString data_path_;
    AlgorithmType algorithm_;
    double runtime_limit_;
    int u_penalty_;
    int b_penalty_;
    bool merge_enabled_;
    double big_order_threshold_;
    int machine_capacity_;

    // CPLEX parameters
    QString cplex_workdir_;
    int cplex_workmem_;
    int cplex_threads_;

    // RF parameters
    int rf_window_;
    int rf_step_;
    double rf_time_;
    int rf_retries_;

    // FO parameters
    int fo_window_;
    int fo_step_;
    int fo_rounds_;
    int fo_buffer_;
    double fo_time_;

    // RR parameters
    double rr_capacity_;
    double rr_bonus_;

    // LR parameters
    int lr_max_iter_;
    double lr_alpha0_;
    double lr_decay_;
    double lr_tol_;

    // Instance info for output filename
    int inst_n_;
    int inst_t_;
    int inst_g_;
    int inst_f_;
    double inst_difficulty_;

    QProcess* solver_process_;
    QTimer* log_reader_;
    QString log_file_path_;
    qint64 log_file_pos_;

    std::atomic<bool> cancel_requested_;
};

#endif  // SOLVER_WORKER_H_
