// main_window.h - Main Window Declaration

#ifndef MAIN_WINDOW_H_
#define MAIN_WINDOW_H_

#include <QMainWindow>
#include <QThread>
#include <QString>

class ParameterWidget;
class ResultsWidget;
class LogWidget;
class SolverWorker;
class QLineEdit;
class QLabel;
class QPushButton;
class QProgressBar;
class QGroupBox;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

signals:
    void StartSolver();

private slots:
    void OnBrowseFile();
    void OnStartOptimization();
    void OnCancelOptimization();
    void OnExportResults();

    // Slots for worker signals
    void OnDataLoaded(int items, int periods, int flows, int groups);
    void OnStageStarted(int stage, const QString& name);
    void OnStageProgress(int stage, int percent, double elapsed);
    void OnStageCompleted(int stage, double objective, double runtime, double gap);
    void OnOptimizationFinished(bool success, const QString& message);
    void OnLogMessage(const QString& message);

private:
    void SetupUi();
    void SetupMenuBar();
    void SetupConnections();
    void UpdateUiState(bool is_running);
    void ResetProgress();

    // File selection
    QGroupBox* file_group_;
    QPushButton* browse_button_;
    QLineEdit* file_path_edit_;
    QLabel* file_info_label_;

    // Parameters
    ParameterWidget* param_widget_;

    // Run control
    QPushButton* start_button_;
    QPushButton* cancel_button_;
    QLabel* status_label_;

    // Progress
    QGroupBox* progress_group_;
    QLabel* stage_labels_[4];
    QProgressBar* progress_bars_[4];
    QLabel* time_labels_[4];

    // Results and log
    ResultsWidget* results_widget_;
    LogWidget* log_widget_;

    // Export
    QPushButton* export_button_;
    QLabel* total_time_label_;

    // Worker thread
    QThread* worker_thread_;
    SolverWorker* solver_worker_;

    // State
    bool is_running_;
    QString current_file_path_;
    double total_runtime_;
};

#endif  // MAIN_WINDOW_H_
