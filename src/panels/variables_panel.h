// variables_panel.h - Variables browser panel
// Displays decision variables in a table with export functionality

#ifndef VARIABLES_PANEL_H_
#define VARIABLES_PANEL_H_

#include <QWidget>
#include <QJsonObject>
#include <QJsonArray>

class QButtonGroup;
class QTableWidget;
class QPushButton;
class QLabel;

class VariablesPanel : public QWidget {
    Q_OBJECT

public:
    explicit VariablesPanel(QWidget* parent = nullptr);

    void LoadData(const QJsonObject& json);
    void Clear();

private slots:
    void OnVariableButtonClicked(int id);
    void OnExportCsv();

private:
    void SetupUi();
    void DisplayVariable(const QString& name);
    void PopulateTable2D(const QJsonArray& data, int rows, int cols,
                         const QString& row_prefix, const QString& col_prefix);
    void PopulateTable1D(const QJsonArray& data, const QString& row_prefix);
    QColor GetHighlightColor() const;

    QButtonGroup* var_button_group_;
    QVector<QPushButton*> var_buttons_;
    QTableWidget* table_;
    QPushButton* export_button_;
    QLabel* info_label_;

    QJsonObject variables_;
    QString current_var_;
    QStringList var_names_;
};

#endif  // VARIABLES_PANEL_H_
