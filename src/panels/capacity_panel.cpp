// capacity_panel.cpp - Capacity utilization panel implementation

#include "capacity_panel.h"
#include "../widgets/line_chart.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QLabel>
#include <QJsonArray>

CapacityPanel::CapacityPanel(QWidget* parent)
    : QWidget(parent) {
    SetupUi();
}

void CapacityPanel::SetupUi() {
    auto* layout = new QVBoxLayout(this);
    layout->setSpacing(12);
    layout->setContentsMargins(8, 8, 8, 8);

    // Summary stats
    auto* stats_group = new QGroupBox(QString::fromUtf8("产能利用率统计"), this);
    auto* stats_layout = new QHBoxLayout(stats_group);

    avg_label_ = new QLabel("--", this);
    avg_label_->setStyleSheet("font-size: 11pt;");
    max_label_ = new QLabel("--", this);
    max_label_->setStyleSheet("font-size: 11pt;");

    stats_layout->addWidget(new QLabel(QString::fromUtf8("平均利用率:"), this));
    stats_layout->addWidget(avg_label_);
    stats_layout->addSpacing(30);
    stats_layout->addWidget(new QLabel(QString::fromUtf8("最大利用率:"), this));
    stats_layout->addWidget(max_label_);
    stats_layout->addStretch();

    layout->addWidget(stats_group);

    // Chart
    auto* chart_group = new QGroupBox(QString::fromUtf8("各周期产能利用率"), this);
    auto* chart_layout = new QVBoxLayout(chart_group);

    chart_ = new LineChart(this);
    chart_->SetYRange(0.0, 1.0);
    chart_->SetAxisLabels(QString::fromUtf8("周期"), "");
    chart_layout->addWidget(chart_);

    layout->addWidget(chart_group, 1);
}

void CapacityPanel::LoadData(const QJsonObject& json) {
    QJsonObject metrics = json["metrics"].toObject();
    QJsonObject capacity = metrics["capacity"].toObject();

    double avg = capacity["avg_utilization"].toDouble();
    double max = capacity["max_utilization"].toDouble();
    QJsonArray by_period = capacity["by_period"].toArray();

    avg_label_->setText(QString("%1%").arg(avg * 100, 0, 'f', 1));
    max_label_->setText(QString("%1%").arg(max * 100, 0, 'f', 1));

    // Find max period
    int max_period = 0;
    double max_val = 0.0;
    QVector<double> values;
    for (int i = 0; i < by_period.size(); ++i) {
        double val = by_period[i].toDouble();
        values.append(val);
        if (val > max_val) {
            max_val = val;
            max_period = i + 1;
        }
    }

    if (max_period > 0) {
        max_label_->setText(QString("%1% (%2 %3)")
            .arg(max * 100, 0, 'f', 1)
            .arg(QString::fromUtf8("周期"))
            .arg(max_period));
    }

    chart_->SetData(values);

    // Add reference line at 100%
    chart_->SetReferenceLine(1.0, "100%");
}

void CapacityPanel::Clear() {
    avg_label_->setText("--");
    max_label_->setText("--");
    chart_->Clear();
}
