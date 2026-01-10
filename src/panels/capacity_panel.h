// capacity_panel.h - Capacity utilization panel
// Displays capacity utilization statistics and chart

#ifndef CAPACITY_PANEL_H_
#define CAPACITY_PANEL_H_

#include <QWidget>
#include <QJsonObject>

class LineChart;
class QLabel;
class QGroupBox;

class CapacityPanel : public QWidget {
    Q_OBJECT

public:
    explicit CapacityPanel(QWidget* parent = nullptr);

    void LoadData(const QJsonObject& json);
    void Clear();

private:
    void SetupUi();

    QLabel* avg_label_;
    QLabel* max_label_;
    LineChart* chart_;
};

#endif  // CAPACITY_PANEL_H_
