// parameter_widget.cpp - Parameter Configuration Widget Implementation

#include "parameter_widget.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QComboBox>
#include <QCheckBox>
#include <QPushButton>
#include <QLabel>
#include <QGroupBox>

ParameterWidget::ParameterWidget(QWidget* parent)
    : QGroupBox(QString::fromUtf8("参数设置"), parent) {
    SetupUi();
    ResetDefaults();
}

void ParameterWidget::SetupUi() {
    auto* layout = new QVBoxLayout(this);

    SetupBasicParams(layout);
    SetupAdvancedParams(layout);

    // Reset button
    reset_button_ = new QPushButton(QString::fromUtf8("重置"), this);
    connect(reset_button_, &QPushButton::clicked, this, &ParameterWidget::ResetDefaults);
    layout->addWidget(reset_button_);

    layout->addStretch();
}

void ParameterWidget::SetupBasicParams(QVBoxLayout* layout) {
    auto* form = new QFormLayout();
    form->setLabelAlignment(Qt::AlignRight);

    // Algorithm Selection
    algorithm_combo_ = new QComboBox(this);
    algorithm_combo_->addItem(QString::fromUtf8("RF  - 滚动松弛固定"), 0);
    algorithm_combo_->addItem(QString::fromUtf8("RFO - RF + 滑动窗口优化"), 1);
    algorithm_combo_->addItem(QString::fromUtf8("RR  - 三阶段分解"), 2);
    form->addRow(QString::fromUtf8("算法"), algorithm_combo_);

    // CPLEX Runtime Limit
    runtime_limit_spin_ = new QDoubleSpinBox(this);
    runtime_limit_spin_->setRange(1.0, 3600.0);
    runtime_limit_spin_->setSuffix(" s");
    runtime_limit_spin_->setDecimals(1);
    runtime_limit_spin_->setSingleStep(10.0);
    form->addRow(QString::fromUtf8("CPLEX时限"), runtime_limit_spin_);

    // Machine Capacity
    machine_capacity_spin_ = new QSpinBox(this);
    machine_capacity_spin_->setRange(720, 2880);
    machine_capacity_spin_->setSingleStep(60);
    form->addRow(QString::fromUtf8("机器日产能"), machine_capacity_spin_);

    // Unmet Penalty
    u_penalty_spin_ = new QSpinBox(this);
    u_penalty_spin_->setRange(1, 1000000);
    u_penalty_spin_->setSingleStep(1000);
    form->addRow(QString::fromUtf8("未满足惩罚"), u_penalty_spin_);

    // Backlog Penalty
    b_penalty_spin_ = new QSpinBox(this);
    b_penalty_spin_->setRange(1, 100000);
    b_penalty_spin_->setSingleStep(10);
    form->addRow(QString::fromUtf8("缺货惩罚"), b_penalty_spin_);

    // Big Order Threshold
    big_order_threshold_spin_ = new QDoubleSpinBox(this);
    big_order_threshold_spin_->setRange(0.0, 100000.0);
    big_order_threshold_spin_->setDecimals(1);
    big_order_threshold_spin_->setSingleStep(100.0);
    form->addRow(QString::fromUtf8("合并阈值"), big_order_threshold_spin_);

    // Merge Checkbox
    merge_checkbox_ = new QCheckBox(this);
    form->addRow(QString::fromUtf8("启用订单合并"), merge_checkbox_);

    layout->addLayout(form);

    // Connect signals
    connect(algorithm_combo_, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &ParameterWidget::OnAlgorithmChanged);
    connect(merge_checkbox_, &QCheckBox::toggled, big_order_threshold_spin_, &QWidget::setEnabled);
}

void ParameterWidget::SetupAdvancedParams(QVBoxLayout* layout) {
    // Advanced toggle button
    advanced_toggle_ = new QPushButton(QString::fromUtf8("▶ 高级算法参数"), this);
    advanced_toggle_->setCheckable(true);
    advanced_toggle_->setChecked(false);
    advanced_toggle_->setStyleSheet("QPushButton { text-align: left; padding: 5px; }");
    layout->addWidget(advanced_toggle_);

    // Advanced container
    advanced_container_ = new QWidget(this);
    auto* advanced_layout = new QVBoxLayout(advanced_container_);
    advanced_layout->setContentsMargins(0, 0, 0, 0);

    // RF Parameters Group
    rf_group_ = new QGroupBox(QString::fromUtf8("RF参数"), advanced_container_);
    auto* rf_form = new QFormLayout(rf_group_);
    rf_form->setLabelAlignment(Qt::AlignRight);

    rf_window_spin_ = new QSpinBox(rf_group_);
    rf_window_spin_->setRange(3, 15);
    rf_window_spin_->setToolTip(QString::fromUtf8("RF窗口大小 W，控制每次求解的时间跨度"));
    rf_form->addRow(QString::fromUtf8("窗口大小"), rf_window_spin_);

    rf_step_spin_ = new QSpinBox(rf_group_);
    rf_step_spin_->setRange(1, 5);
    rf_step_spin_->setToolTip(QString::fromUtf8("RF固定步长 S，每次固定变量的数量"));
    rf_form->addRow(QString::fromUtf8("固定步长"), rf_step_spin_);

    rf_time_spin_ = new QDoubleSpinBox(rf_group_);
    rf_time_spin_->setRange(10.0, 300.0);
    rf_time_spin_->setSuffix(" s");
    rf_time_spin_->setDecimals(1);
    rf_time_spin_->setToolTip(QString::fromUtf8("RF子问题求解时间限制"));
    rf_form->addRow(QString::fromUtf8("子问题时限"), rf_time_spin_);

    rf_retries_spin_ = new QSpinBox(rf_group_);
    rf_retries_spin_->setRange(1, 10);
    rf_retries_spin_->setToolTip(QString::fromUtf8("RF窗口扩展失败时的最大重试次数"));
    rf_form->addRow(QString::fromUtf8("最大重试"), rf_retries_spin_);

    advanced_layout->addWidget(rf_group_);

    // FO Parameters Group
    fo_group_ = new QGroupBox(QString::fromUtf8("FO参数 (RFO专用)"), advanced_container_);
    auto* fo_form = new QFormLayout(fo_group_);
    fo_form->setLabelAlignment(Qt::AlignRight);

    fo_window_spin_ = new QSpinBox(fo_group_);
    fo_window_spin_->setRange(4, 20);
    fo_window_spin_->setToolTip(QString::fromUtf8("FO优化窗口大小"));
    fo_form->addRow(QString::fromUtf8("窗口大小"), fo_window_spin_);

    fo_step_spin_ = new QSpinBox(fo_group_);
    fo_step_spin_->setRange(1, 8);
    fo_step_spin_->setToolTip(QString::fromUtf8("FO窗口滑动步长"));
    fo_form->addRow(QString::fromUtf8("滑动步长"), fo_step_spin_);

    fo_rounds_spin_ = new QSpinBox(fo_group_);
    fo_rounds_spin_->setRange(1, 5);
    fo_rounds_spin_->setToolTip(QString::fromUtf8("FO优化最大轮数"));
    fo_form->addRow(QString::fromUtf8("最大轮数"), fo_rounds_spin_);

    fo_buffer_spin_ = new QSpinBox(fo_group_);
    fo_buffer_spin_->setRange(0, 3);
    fo_buffer_spin_->setToolTip(QString::fromUtf8("FO边界缓冲宽度"));
    fo_form->addRow(QString::fromUtf8("边界缓冲"), fo_buffer_spin_);

    fo_time_spin_ = new QDoubleSpinBox(fo_group_);
    fo_time_spin_->setRange(10.0, 120.0);
    fo_time_spin_->setSuffix(" s");
    fo_time_spin_->setDecimals(1);
    fo_time_spin_->setToolTip(QString::fromUtf8("FO子问题求解时间限制"));
    fo_form->addRow(QString::fromUtf8("子问题时限"), fo_time_spin_);

    advanced_layout->addWidget(fo_group_);

    // RR Parameters Group
    rr_group_ = new QGroupBox(QString::fromUtf8("RR参数 (RR专用)"), advanced_container_);
    auto* rr_form = new QFormLayout(rr_group_);
    rr_form->setLabelAlignment(Qt::AlignRight);

    rr_capacity_spin_ = new QDoubleSpinBox(rr_group_);
    rr_capacity_spin_->setRange(1.0, 2.0);
    rr_capacity_spin_->setDecimals(2);
    rr_capacity_spin_->setSingleStep(0.1);
    rr_capacity_spin_->setToolTip(QString::fromUtf8("阶段1产能放大系数，用于求解启动结构"));
    rr_form->addRow(QString::fromUtf8("产能放大"), rr_capacity_spin_);

    rr_bonus_spin_ = new QDoubleSpinBox(rr_group_);
    rr_bonus_spin_->setRange(0.0, 200.0);
    rr_bonus_spin_->setDecimals(1);
    rr_bonus_spin_->setSingleStep(10.0);
    rr_bonus_spin_->setToolTip(QString::fromUtf8("连续启动奖励系数，鼓励形成跨期机会"));
    rr_form->addRow(QString::fromUtf8("连续奖励"), rr_bonus_spin_);

    advanced_layout->addWidget(rr_group_);

    advanced_container_->setVisible(false);
    layout->addWidget(advanced_container_);

    // Connect toggle
    connect(advanced_toggle_, &QPushButton::toggled, this, &ParameterWidget::ToggleAdvancedParams);
}

void ParameterWidget::OnAlgorithmChanged(int index) {
    UpdateParamGroupStates(index);
    emit AlgorithmChanged(index);
}

void ParameterWidget::ToggleAdvancedParams(bool expanded) {
    advanced_container_->setVisible(expanded);
    advanced_toggle_->setText(expanded
        ? QString::fromUtf8("▼ 高级算法参数")
        : QString::fromUtf8("▶ 高级算法参数"));

    if (expanded) {
        UpdateParamGroupStates(algorithm_combo_->currentIndex());
    }
}

void ParameterWidget::UpdateParamGroupStates(int algorithmIndex) {
    // RF: index 0, RFO: index 1, RR: index 2
    bool rf_enabled = (algorithmIndex == 0 || algorithmIndex == 1);
    bool fo_enabled = (algorithmIndex == 1);
    bool rr_enabled = (algorithmIndex == 2);

    rf_group_->setEnabled(rf_enabled);
    fo_group_->setEnabled(fo_enabled);
    rr_group_->setEnabled(rr_enabled);
}

void ParameterWidget::ResetDefaults() {
    // Basic parameters
    algorithm_combo_->setCurrentIndex(0);  // RF
    runtime_limit_spin_->setValue(30.0);
    machine_capacity_spin_->setValue(1440);
    u_penalty_spin_->setValue(10000);
    b_penalty_spin_->setValue(100);
    merge_checkbox_->setChecked(true);
    big_order_threshold_spin_->setValue(1000.0);
    big_order_threshold_spin_->setEnabled(true);

    // RF parameters
    rf_window_spin_->setValue(6);
    rf_step_spin_->setValue(1);
    rf_time_spin_->setValue(60.0);
    rf_retries_spin_->setValue(3);

    // FO parameters
    fo_window_spin_->setValue(8);
    fo_step_spin_->setValue(3);
    fo_rounds_spin_->setValue(2);
    fo_buffer_spin_->setValue(1);
    fo_time_spin_->setValue(30.0);

    // RR parameters
    rr_capacity_spin_->setValue(1.2);
    rr_bonus_spin_->setValue(50.0);

    UpdateParamGroupStates(0);
}

// Basic parameter getters
int ParameterWidget::GetAlgorithmIndex() const {
    return algorithm_combo_->currentIndex();
}

double ParameterWidget::GetRuntimeLimit() const {
    return runtime_limit_spin_->value();
}

int ParameterWidget::GetMachineCapacity() const {
    return machine_capacity_spin_->value();
}

int ParameterWidget::GetUPenalty() const {
    return u_penalty_spin_->value();
}

int ParameterWidget::GetBPenalty() const {
    return b_penalty_spin_->value();
}

bool ParameterWidget::GetMergeEnabled() const {
    return merge_checkbox_->isChecked();
}

double ParameterWidget::GetBigOrderThreshold() const {
    return big_order_threshold_spin_->value();
}

// RF parameter getters
int ParameterWidget::GetRFWindow() const {
    return rf_window_spin_->value();
}

int ParameterWidget::GetRFStep() const {
    return rf_step_spin_->value();
}

double ParameterWidget::GetRFTime() const {
    return rf_time_spin_->value();
}

int ParameterWidget::GetRFRetries() const {
    return rf_retries_spin_->value();
}

// FO parameter getters
int ParameterWidget::GetFOWindow() const {
    return fo_window_spin_->value();
}

int ParameterWidget::GetFOStep() const {
    return fo_step_spin_->value();
}

int ParameterWidget::GetFORounds() const {
    return fo_rounds_spin_->value();
}

int ParameterWidget::GetFOBuffer() const {
    return fo_buffer_spin_->value();
}

double ParameterWidget::GetFOTime() const {
    return fo_time_spin_->value();
}

// RR parameter getters
double ParameterWidget::GetRRCapacity() const {
    return rr_capacity_spin_->value();
}

double ParameterWidget::GetRRBonus() const {
    return rr_bonus_spin_->value();
}
