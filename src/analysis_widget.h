// analysis_widget.h - Main analysis widget for result analysis tab
// Loads JSON result files and displays analysis in multiple panels

#ifndef ANALYSIS_WIDGET_H_
#define ANALYSIS_WIDGET_H_

#include <QWidget>
#include <QJsonObject>
#include <QString>

class QPushButton;
class QLabel;
class QTabWidget;
class OverviewPanel;
class CapacityPanel;
class SetupPanel;
class VariablesPanel;

class AnalysisWidget : public QWidget {
    Q_OBJECT

public:
    explicit AnalysisWidget(QWidget* parent = nullptr);

    // Load a JSON result file
    bool LoadJsonFile(const QString& path);

    // Clear all data
    void Clear();

    // Check if data is loaded
    bool HasData() const { return !json_data_.isEmpty(); }

private slots:
    void OnOpenFile();
    void OnClearData();

private:
    void SetupUi();
    void UpdateAllPanels();

    // Top controls
    QPushButton* open_button_;
    QPushButton* clear_button_;
    QLabel* file_label_;

    // Tab widget for different views
    QTabWidget* tabs_;

    // Panels
    OverviewPanel* overview_panel_;
    CapacityPanel* capacity_panel_;
    SetupPanel* setup_panel_;
    VariablesPanel* variables_panel_;

    // Data
    QJsonObject json_data_;
    QString current_file_;
};

#endif  // ANALYSIS_WIDGET_H_
