// setup_panel.h - Setup/Carryover visualization panel
// Displays Y and L matrices as heatmaps

#ifndef SETUP_PANEL_H_
#define SETUP_PANEL_H_

#include <QWidget>
#include <QJsonObject>

class Heatmap;
class QButtonGroup;
class QLabel;
class QScrollArea;

class SetupPanel : public QWidget {
    Q_OBJECT

public:
    explicit SetupPanel(QWidget* parent = nullptr);

    void LoadData(const QJsonObject& json);
    void Clear();

private slots:
    void OnMatrixChanged(int id);

private:
    void SetupUi();
    void DisplayMatrix(const QString& name);

    QButtonGroup* matrix_group_;
    QLabel* info_label_;
    QScrollArea* scroll_area_;
    Heatmap* heatmap_;

    QJsonObject variables_;
};

#endif  // SETUP_PANEL_H_
