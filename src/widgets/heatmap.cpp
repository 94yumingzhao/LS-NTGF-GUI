// heatmap.cpp - Heatmap widget implementation

#include "heatmap.h"
#include <QPainter>
#include <QPaintEvent>

Heatmap::Heatmap(QWidget* parent)
    : QWidget(parent)
    , zero_color_(QColor("#f1f3f5"))
    , one_color_(QColor("#228be6")) {
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setMinimumSize(150, 100);
}

void Heatmap::SetData(const QVector<QVector<int>>& data) {
    data_ = data;
    updateGeometry();
    update();
}

void Heatmap::SetColors(const QColor& zero_color, const QColor& one_color) {
    zero_color_ = zero_color;
    one_color_ = one_color;
    update();
}

void Heatmap::SetLabels(const QString& row_label, const QString& col_label) {
    row_label_ = row_label;
    col_label_ = col_label;
    update();
}

void Heatmap::Clear() {
    data_.clear();
    updateGeometry();
    update();
}

QSize Heatmap::sizeHint() const {
    // 返回一个合理的默认尺寸，实际尺寸由布局决定
    return QSize(400, 300);
}

void Heatmap::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // Background
    painter.fillRect(rect(), Qt::white);

    if (data_.isEmpty()) {
        painter.setPen(QColor("#adb5bd"));
        painter.drawText(rect(), Qt::AlignCenter, QString::fromUtf8("--"));
        return;
    }

    int rows = data_.size();
    int cols = data_[0].size();

    // 根据可用空间计算单元格大小
    int availableW = width() - kMarginLeft - kMarginRight;
    int availableH = height() - kMarginTop - kMarginBottom;

    int cellW = availableW / cols;
    int cellH = availableH / rows;

    // 限制单元格大小范围
    cellW = qBound(kMinCellSize, cellW, kMaxCellSize);
    cellH = qBound(kMinCellSize, cellH, kMaxCellSize);

    // Draw column headers (period numbers)
    painter.setPen(QColor("#6c757d"));
    painter.setFont(QFont("", 7));
    int labelStep = cols > 20 ? 5 : (cols > 10 ? 2 : 1);
    for (int c = 0; c < cols; c += labelStep) {
        int x = kMarginLeft + c * cellW + cellW / 2;
        painter.drawText(QRect(x - 10, 2, 20, kMarginTop - 4),
                         Qt::AlignCenter, QString::number(c + 1));
    }

    // Draw row headers (group numbers) and cells
    for (int r = 0; r < rows; ++r) {
        int y = kMarginTop + r * cellH;

        // Row label
        painter.setPen(QColor("#6c757d"));
        painter.drawText(QRect(2, y, kMarginLeft - 4, cellH),
                         Qt::AlignRight | Qt::AlignVCenter,
                         QString("G%1").arg(r + 1));

        // Cells
        for (int c = 0; c < cols && c < data_[r].size(); ++c) {
            int x = kMarginLeft + c * cellW;
            int val = data_[r][c];

            painter.setPen(Qt::NoPen);
            painter.setBrush(val > 0 ? one_color_ : zero_color_);
            painter.drawRect(x, y, cellW - 1, cellH - 1);
        }
    }

    // Draw legend
    int legendY = height() - kMarginBottom + 4;
    painter.setFont(QFont("", 8));

    painter.setPen(Qt::NoPen);
    painter.setBrush(zero_color_);
    painter.drawRect(kMarginLeft, legendY, 12, 12);
    painter.setPen(QColor("#495057"));
    painter.drawText(kMarginLeft + 16, legendY + 10, "= 0");

    painter.setPen(Qt::NoPen);
    painter.setBrush(one_color_);
    painter.drawRect(kMarginLeft + 50, legendY, 12, 12);
    painter.setPen(QColor("#495057"));
    painter.drawText(kMarginLeft + 66, legendY + 10, "= 1");
}
