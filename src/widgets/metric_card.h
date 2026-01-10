// metric_card.h - Metric display card widget
// Shows a label and value in a styled card format

#ifndef METRIC_CARD_H_
#define METRIC_CARD_H_

#include <QFrame>
#include <QString>

class QLabel;

class MetricCard : public QFrame {
    Q_OBJECT

public:
    explicit MetricCard(const QString& title, QWidget* parent = nullptr);

    void SetValue(const QString& value);
    void SetValue(double value, int precision = 2);
    void SetValue(int value);
    void SetTitle(const QString& title);
    void SetHighlight(bool highlight);

private:
    void SetupUi();

    QLabel* title_label_;
    QLabel* value_label_;
    QString title_;
};

#endif  // METRIC_CARD_H_
