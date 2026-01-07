// main_window.h - Main Window Declaration
// Layout: Left sidebar (file, params, controls, results) + Right log area

#ifndef MAIN_WINDOW_H_
#define MAIN_WINDOW_H_

#include <QMainWindow>
#include <QThread>
#include <QString>
#include "difficulty_mapper.h"

class ParameterWidget;
class ResultsWidget;
class LogWidget;
class CplexSettingsWidget;
class SolverWorker;
class GeneratorWidget;
class GeneratorWorker;
class QLineEdit;
class QLabel;
class QPushButton;
class QGroupBox;
class QSplitter;
class QTabWidget;

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
    void OnExportLog();
    void OnAlgorithmChanged(int index);

    // Slots for solver worker signals
    void OnDataLoaded(int items, int periods, int flows, int groups);
    void OnOrdersMerged(int original, int merged);
    void OnMergeSkipped();
    void OnStageStarted(int stage, const QString& name);
    void OnStageCompleted(int stage, double objective, double runtime, double gap);
    void OnOptimizationFinished(bool success, const QString& message);
    void OnLogMessage(const QString& message);

    // Slots for generator
    void OnGenerateRequested(const GeneratorConfig& config);
    void OnGenerationStarted(int count);
    void OnInstanceGenerated(int index, const QString& filename);
    void OnGenerationFinished(bool success, const QString& message, const QStringList& files);

private:
    void SetupUi();
    void SetupMenuBar();
    void SetupConnections();
    void UpdateUiState(bool is_running);
    void ResetState();
    void ParseCsvForIndicators(const QString& path);

    // Main layout
    QSplitter* main_splitter_;

    // Left sidebar - File selection
    QGroupBox* file_group_;
    QPushButton* browse_button_;
    QLineEdit* file_path_edit_;
    QLabel* file_info_label_;

    // Left sidebar - Parameters
    ParameterWidget* param_widget_;

    // Left sidebar - Run control
    QGroupBox* control_group_;
    QPushButton* start_button_;
    QPushButton* cancel_button_;
    QLabel* status_label_;

    // Left sidebar - Results summary
    ResultsWidget* results_widget_;

    // Left sidebar - Export
    QPushButton* export_button_;

    // Right area - CPLEX settings (above log)
    CplexSettingsWidget* cplex_settings_widget_;

    // Right area - Log
    LogWidget* log_widget_;

    // Left sidebar - Generator tab
    GeneratorWidget* generator_widget_;

    // Tab widget for mode switching
    QTabWidget* mode_tabs_;

    // Worker threads
    QThread* solver_thread_;
    SolverWorker* solver_worker_;
    QThread* generator_thread_;
    GeneratorWorker* generator_worker_;

    // State
    bool is_running_;
    QString current_file_path_;
    double total_runtime_;

    // Instance info (parsed from CSV)
    int inst_n_;
    int inst_t_;
    int inst_g_;
    int inst_f_;
    double inst_difficulty_;
};

#endif  // MAIN_WINDOW_H_
