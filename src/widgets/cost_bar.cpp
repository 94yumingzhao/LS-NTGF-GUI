// cost_bar.cpp - Horizontal stacked bar chart implementation

#include "cost_bar.h"
#include <QPainter>
#include <QPaintEvent>

CostBar::CostBar(QWidget* parent)
    : QWidget(parent)
    , total_(0.0) {
    setMinimumHeight(kBarHeight + kLegendHeight + 8);
    setMaximumHeight(kBarHeight + kLegendHeight + 16);
}

void CostBar::SetCosts(double production, double setup, double inventory,
                       double backorder, double unmet) {
    items_.clear();
    total_ = production + setup + inventory + backorder + unmet;

    if (total_ > 0) {
        items_.append({QString::fromUtf8("生产"), production, QColor("#4dabf7")});
        items_.append({QString::fromUtf8("启动"), setup, QColor("#69db7c")});
        items_.append({QString::fromUtf8("库存"), inventory, QColor("#ffd43b")});
        items_.append({QString::fromUtf8("欠交"), backorder, QColor("#ff922b")});
        items_.append({QString::fromUtf8("未满足"), unmet, QColor("#ff6b6b")});
    }

    update();
}

void CostBar::Clear() {
    items_.clear();
    total_ = 0.0;
    update();
}

void CostBar::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    int w = width();
    int barY = 4;

    if (total_ <= 0 || items_.isEmpty()) {
        // Draw empty state
        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor("#e9ecef"));
        painter.drawRoundedRect(0, barY, w, kBarHeight, 4, 4);

        painter.setPen(QColor("#adb5bd"));
        painter.drawText(QRect(0, barY, w, kBarHeight), Qt::AlignCenter,
                         QString::fromUtf8("--"));
        return;
    }

    // Draw stacked bar
    int x = 0;
    for (int i = 0; i < items_.size(); ++i) {
        const auto& item = items_[i];
        int segmentWidth = static_cast<int>((item.value / total_) * w);

        // Last segment takes remaining width
        if (i == items_.size() - 1) {
            segmentWidth = w - x;
        }

        if (segmentWidth > 0) {
            painter.setPen(Qt::NoPen);
            painter.setBrush(item.color);

            if (i == 0) {
                // First segment: rounded left corners
                painter.drawRoundedRect(x, barY, segmentWidth + 4, kBarHeight, 4, 4);
                painter.drawRect(x + segmentWidth - 4, barY, 8, kBarHeight);
            } else if (i == items_.size() - 1) {
                // Last segment: rounded right corners
                painter.drawRoundedRect(x - 4, barY, segmentWidth + 4, kBarHeight, 4, 4);
                painter.drawRect(x - 4, barY, 8, kBarHeight);
            } else {
                // Middle segments: no rounding
                painter.drawRect(x, barY, segmentWidth, kBarHeight);
            }

            x += segmentWidth;
        }
    }

    // Draw legend
    int legendY = barY + kBarHeight + 6;
    x = 0;
    painter.setFont(QFont("", 8));

    for (const auto& item : items_) {
        if (item.value <= 0) continue;

        double pct = (item.value / total_) * 100.0;
        QString label = QString("%1 %2%").arg(item.name).arg(pct, 0, 'f', 1);

        // Color box
        painter.setPen(Qt::NoPen);
        painter.setBrush(item.color);
        painter.drawRect(x, legendY + 2, 10, 10);

        // Label
        painter.setPen(QColor("#495057"));
        QRect textRect(x + 14, legendY, 80, kLegendHeight);
        painter.drawText(textRect, Qt::AlignLeft | Qt::AlignVCenter, label);

        x += 75;
    }
}
