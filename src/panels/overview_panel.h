// overview_panel.h - Overview panel for result analysis
// Displays key metrics, cost breakdown, and problem info

#ifndef OVERVIEW_PANEL_H_
#define OVERVIEW_PANEL_H_

#include <QWidget>
#include <QJsonObject>

class MetricCard;
class CostBar;
class QLabel;
class QGroupBox;

class OverviewPanel : public QWidget {
    Q_OBJECT

public:
    explicit OverviewPanel(QWidget* parent = nullptr);

    void LoadData(const QJsonObject& json);
    void Clear();

private:
    void SetupUi();

    // Metric cards
    MetricCard* card_objective_;
    MetricCard* card_time_;
    MetricCard* card_gap_;
    MetricCard* card_unmet_;
    MetricCard* card_setups_;
    MetricCard* card_carryovers_;

    // Cost breakdown
    QGroupBox* cost_group_;
    CostBar* cost_bar_;

    // Problem info
    QGroupBox* problem_group_;
    QLabel* problem_label_;

    // Algorithm-specific metrics
    QGroupBox* algo_group_;
    QLabel* algo_label_;
};

#endif  // OVERVIEW_PANEL_H_
