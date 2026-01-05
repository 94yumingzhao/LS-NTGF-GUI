// parameter_widget.cpp - Parameter Configuration Widget Implementation

#include "parameter_widget.h"

#include <QVBoxLayout>
#include <QFormLayout>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QPushButton>
#include <QLabel>

ParameterWidget::ParameterWidget(QWidget* parent)
    : QGroupBox(QString::fromUtf8("参数设置"), parent) {
    SetupUi();
    ResetDefaults();
}

void ParameterWidget::SetupUi() {
    auto* layout = new QVBoxLayout(this);

    auto* form = new QFormLayout();
    form->setLabelAlignment(Qt::AlignRight);

    // CPLEX Runtime Limit
    runtime_limit_spin_ = new QDoubleSpinBox(this);
    runtime_limit_spin_->setRange(1.0, 3600.0);
    runtime_limit_spin_->setSuffix(" s");
    runtime_limit_spin_->setDecimals(1);
    runtime_limit_spin_->setSingleStep(10.0);
    form->addRow(QString::fromUtf8("CPLEX时限:"), runtime_limit_spin_);

    // Unmet Penalty
    u_penalty_spin_ = new QSpinBox(this);
    u_penalty_spin_->setRange(1, 1000000);
    u_penalty_spin_->setSingleStep(1000);
    form->addRow(QString::fromUtf8("未满足惩罚:"), u_penalty_spin_);

    // Backlog Penalty
    b_penalty_spin_ = new QSpinBox(this);
    b_penalty_spin_->setRange(1, 100000);
    b_penalty_spin_->setSingleStep(10);
    form->addRow(QString::fromUtf8("缺货惩罚:"), b_penalty_spin_);

    // Big Order Threshold
    big_order_threshold_spin_ = new QDoubleSpinBox(this);
    big_order_threshold_spin_->setRange(0.0, 100000.0);
    big_order_threshold_spin_->setDecimals(1);
    big_order_threshold_spin_->setSingleStep(100.0);
    form->addRow(QString::fromUtf8("大订单阈值:"), big_order_threshold_spin_);

    layout->addLayout(form);

    // Reset button
    reset_button_ = new QPushButton(QString::fromUtf8("重置默认"), this);
    connect(reset_button_, &QPushButton::clicked, this, &ParameterWidget::ResetDefaults);
    layout->addWidget(reset_button_);

    layout->addStretch();
}

void ParameterWidget::ResetDefaults() {
    runtime_limit_spin_->setValue(30.0);
    u_penalty_spin_->setValue(10000);
    b_penalty_spin_->setValue(100);
    big_order_threshold_spin_->setValue(1000.0);
}

double ParameterWidget::GetRuntimeLimit() const {
    return runtime_limit_spin_->value();
}

int ParameterWidget::GetUPenalty() const {
    return u_penalty_spin_->value();
}

int ParameterWidget::GetBPenalty() const {
    return b_penalty_spin_->value();
}

double ParameterWidget::GetBigOrderThreshold() const {
    return big_order_threshold_spin_->value();
}
