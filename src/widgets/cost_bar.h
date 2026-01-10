// cost_bar.h - Horizontal stacked bar chart for cost breakdown
// Displays cost components as colored segments with labels

#ifndef COST_BAR_H_
#define COST_BAR_H_

#include <QWidget>
#include <QVector>
#include <QString>
#include <QColor>

struct CostItem {
    QString name;
    double value;
    QColor color;
};

class CostBar : public QWidget {
    Q_OBJECT

public:
    explicit CostBar(QWidget* parent = nullptr);

    void SetCosts(double production, double setup, double inventory,
                  double backorder, double unmet);
    void Clear();

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    QVector<CostItem> items_;
    double total_;

    static const int kBarHeight = 24;
    static const int kLegendHeight = 20;
};

#endif  // COST_BAR_H_
