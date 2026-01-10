// heatmap.h - Heatmap widget for matrix visualization
// Displays a 2D matrix as colored cells

#ifndef HEATMAP_H_
#define HEATMAP_H_

#include <QWidget>
#include <QVector>
#include <QString>
#include <QColor>

class Heatmap : public QWidget {
    Q_OBJECT

public:
    explicit Heatmap(QWidget* parent = nullptr);

    // Set data as 2D matrix [rows][cols]
    void SetData(const QVector<QVector<int>>& data);
    void SetColors(const QColor& zero_color, const QColor& one_color);
    void SetLabels(const QString& row_label, const QString& col_label);
    void Clear();

protected:
    void paintEvent(QPaintEvent* event) override;
    QSize sizeHint() const override;

private:
    QVector<QVector<int>> data_;
    QColor zero_color_;
    QColor one_color_;
    QString row_label_;
    QString col_label_;

    static const int kMinCellSize = 8;
    static const int kMaxCellSize = 32;
    static const int kMarginLeft = 35;
    static const int kMarginTop = 20;
    static const int kMarginRight = 10;
    static const int kMarginBottom = 25;  // 留出图例空间
};

#endif  // HEATMAP_H_
