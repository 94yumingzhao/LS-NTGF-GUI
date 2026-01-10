// line_chart.cpp - Simple line chart widget implementation

#include "line_chart.h"
#include <QPainter>
#include <QPainterPath>
#include <QPaintEvent>
#include <cmath>

LineChart::LineChart(QWidget* parent)
    : QWidget(parent)
    , y_min_(0.0)
    , y_max_(100.0)
    , ref_value_(0.0)
    , has_reference_(false) {
    setMinimumHeight(120);
}

void LineChart::SetData(const QVector<double>& values) {
    values_ = values;
    update();
}

void LineChart::SetYRange(double min_val, double max_val) {
    y_min_ = min_val;
    y_max_ = max_val;
    update();
}

void LineChart::SetReferenceLine(double value, const QString& label) {
    ref_value_ = value;
    ref_label_ = label;
    has_reference_ = true;
    update();
}

void LineChart::SetAxisLabels(const QString& x_label, const QString& y_label) {
    x_label_ = x_label;
    y_label_ = y_label;
    update();
}

void LineChart::Clear() {
    values_.clear();
    has_reference_ = false;
    update();
}

void LineChart::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    int w = width();
    int h = height();
    int chartW = w - kMarginLeft - kMarginRight;
    int chartH = h - kMarginTop - kMarginBottom;

    // Background
    painter.fillRect(rect(), Qt::white);

    // Chart area background
    QRect chartRect(kMarginLeft, kMarginTop, chartW, chartH);
    painter.fillRect(chartRect, QColor("#f8f9fa"));

    // Draw horizontal grid lines
    painter.setPen(QPen(QColor("#dee2e6"), 1, Qt::DotLine));
    for (int i = 1; i < 5; ++i) {
        int y = kMarginTop + (chartH * i / 5);
        painter.drawLine(kMarginLeft, y, w - kMarginRight, y);
    }

    // Draw vertical grid lines for each period
    if (!values_.isEmpty() && values_.size() > 1) {
        double xStep = static_cast<double>(chartW) / (values_.size() - 1);
        painter.setPen(QPen(QColor("#e9ecef"), 1, Qt::SolidLine));
        for (int i = 0; i < values_.size(); ++i) {
            int x = kMarginLeft + static_cast<int>(i * xStep);
            painter.drawLine(x, kMarginTop, x, kMarginTop + chartH);
        }
    }

    // Draw Y axis labels
    painter.setPen(QColor("#6c757d"));
    painter.setFont(QFont("", 8));
    for (int i = 0; i <= 5; ++i) {
        int y = kMarginTop + (chartH * i / 5);
        double val = y_max_ - (y_max_ - y_min_) * i / 5;
        QString label = QString::number(val, 'f', 0);
        if (y_max_ <= 1.0) {
            label = QString::number(val * 100, 'f', 0) + "%";
        }
        painter.drawText(QRect(0, y - 8, kMarginLeft - 4, 16),
                         Qt::AlignRight | Qt::AlignVCenter, label);
    }

    if (values_.isEmpty()) {
        painter.setPen(QColor("#adb5bd"));
        painter.drawText(chartRect, Qt::AlignCenter, QString::fromUtf8("--"));
        return;
    }

    // Draw reference line
    if (has_reference_ && ref_value_ >= y_min_ && ref_value_ <= y_max_) {
        double ratio = (ref_value_ - y_min_) / (y_max_ - y_min_);
        int refY = kMarginTop + chartH - static_cast<int>(ratio * chartH);
        painter.setPen(QPen(QColor("#ff6b6b"), 1, Qt::DashLine));
        painter.drawLine(kMarginLeft, refY, w - kMarginRight, refY);

        if (!ref_label_.isEmpty()) {
            painter.setPen(QColor("#ff6b6b"));
            painter.drawText(kMarginLeft + 4, refY - 4, ref_label_);
        }
    }

    // Draw line chart
    if (values_.size() >= 2) {
        QPainterPath path;
        double xStep = static_cast<double>(chartW) / (values_.size() - 1);

        for (int i = 0; i < values_.size(); ++i) {
            double val = qBound(y_min_, values_[i], y_max_);
            double ratio = (val - y_min_) / (y_max_ - y_min_);
            int x = kMarginLeft + static_cast<int>(i * xStep);
            int y = kMarginTop + chartH - static_cast<int>(ratio * chartH);

            if (i == 0) {
                path.moveTo(x, y);
            } else {
                path.lineTo(x, y);
            }
        }

        painter.setPen(QPen(QColor("#228be6"), 2));
        painter.drawPath(path);

        // Draw points
        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor("#228be6"));
        for (int i = 0; i < values_.size(); ++i) {
            double val = qBound(y_min_, values_[i], y_max_);
            double ratio = (val - y_min_) / (y_max_ - y_min_);
            int x = kMarginLeft + static_cast<int>(i * xStep);
            int y = kMarginTop + chartH - static_cast<int>(ratio * chartH);
            painter.drawEllipse(QPoint(x, y), 3, 3);
        }
    }

    // Draw X axis labels
    painter.setPen(QColor("#6c757d"));
    int labelStep = values_.size() > 10 ? values_.size() / 5 : 1;
    double xStep = static_cast<double>(chartW) / (values_.size() - 1);
    for (int i = 0; i < values_.size(); i += labelStep) {
        int x = kMarginLeft + static_cast<int>(i * xStep);
        painter.drawText(QRect(x - 15, h - kMarginBottom + 4, 30, 16),
                         Qt::AlignCenter, QString::number(i + 1));
    }

    // X axis label
    if (!x_label_.isEmpty()) {
        painter.drawText(QRect(kMarginLeft, h - 16, chartW, 16),
                         Qt::AlignRight, x_label_);
    }
}
