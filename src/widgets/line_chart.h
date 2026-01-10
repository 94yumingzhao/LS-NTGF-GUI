// line_chart.h - Simple line chart widget
// Displays a line chart with optional reference line

#ifndef LINE_CHART_H_
#define LINE_CHART_H_

#include <QWidget>
#include <QVector>
#include <QString>

class LineChart : public QWidget {
    Q_OBJECT

public:
    explicit LineChart(QWidget* parent = nullptr);

    void SetData(const QVector<double>& values);
    void SetYRange(double min_val, double max_val);
    void SetReferenceLine(double value, const QString& label = "");
    void SetAxisLabels(const QString& x_label, const QString& y_label);
    void Clear();

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    QVector<double> values_;
    double y_min_;
    double y_max_;
    double ref_value_;
    QString ref_label_;
    QString x_label_;
    QString y_label_;
    bool has_reference_;

    static const int kMarginLeft = 40;
    static const int kMarginRight = 10;
    static const int kMarginTop = 10;
    static const int kMarginBottom = 25;
};

#endif  // LINE_CHART_H_
