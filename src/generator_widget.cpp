// generator_widget.cpp - Instance Generator Widget Implementation

#include "generator_widget.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QLabel>
#include <QComboBox>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QCheckBox>
#include <QPushButton>
#include <QLineEdit>
#include <QRadioButton>
#include <QButtonGroup>
#include <QFileDialog>
#include <QToolButton>
#include <QMessageBox>

GeneratorWidget::GeneratorWidget(QWidget* parent)
    : QWidget(parent) {
    SetupUi();
    SetupConnections();
    UpdatePreview();
}

void GeneratorWidget::SetupUi() {
    auto* main_layout = new QVBoxLayout(this);
    main_layout->setSpacing(8);
    main_layout->setContentsMargins(0, 0, 0, 0);

    // Mode selector
    auto* mode_layout = new QHBoxLayout();
    mode_layout->addWidget(new QLabel(QString::fromUtf8("\u6a21\u5f0f:")));
    mode_combo_ = new QComboBox();
    mode_combo_->addItem(QString::fromUtf8("\u5feb\u901f"));
    mode_combo_->addItem(QString::fromUtf8("\u624b\u52a8"));
    mode_layout->addWidget(mode_combo_);
    mode_layout->addStretch();
    main_layout->addLayout(mode_layout);

    // Quick mode group
    quick_group_ = new QGroupBox(QString::fromUtf8("\u5feb\u901f\u8bbe\u7f6e"));
    SetupQuickModeUi(quick_group_);
    main_layout->addWidget(quick_group_);

    // Manual mode group
    manual_group_ = new QGroupBox(QString::fromUtf8("\u624b\u52a8\u8bbe\u7f6e"));
    SetupManualModeUi(manual_group_);
    manual_group_->setVisible(false);
    main_layout->addWidget(manual_group_);

    // Common settings group
    auto* common_group = new QGroupBox(QString::fromUtf8("\u8f93\u51fa\u8bbe\u7f6e"));
    auto* common_layout = new QFormLayout(common_group);
    common_layout->setSpacing(4);

    seed_spin_ = new QSpinBox();
    seed_spin_->setRange(0, 999999);
    seed_spin_->setValue(0);
    seed_spin_->setSpecialValueText(QString::fromUtf8("\u81ea\u52a8"));
    common_layout->addRow(QString::fromUtf8("\u968f\u673a\u79cd\u5b50:"), seed_spin_);

    count_spin_ = new QSpinBox();
    count_spin_->setRange(1, 100);
    count_spin_->setValue(1);
    common_layout->addRow(QString::fromUtf8("\u751f\u6210\u6570\u91cf:"), count_spin_);

    auto* path_layout = new QHBoxLayout();
    output_edit_ = new QLineEdit();
    output_edit_->setText("D:/YM-Code/LS-NTGF-Data-Cap/data/");
    browse_button_ = new QPushButton("...");
    browse_button_->setFixedWidth(32);
    path_layout->addWidget(output_edit_);
    path_layout->addWidget(browse_button_);
    common_layout->addRow(QString::fromUtf8("\u8f93\u51fa\u8def\u5f84:"), path_layout);

    main_layout->addWidget(common_group);

    // Preview label
    preview_label_ = new QLabel();
    preview_label_->setStyleSheet("background: #f0f0f0; padding: 8px; font-family: monospace; font-size: 9pt;");
    preview_label_->setWordWrap(true);
    main_layout->addWidget(preview_label_);

    // Generate button
    generate_button_ = new QPushButton(QString::fromUtf8("\u751f\u6210"));
    generate_button_->setMinimumHeight(32);
    generate_button_->setStyleSheet("font-weight: bold;");
    main_layout->addWidget(generate_button_);

    main_layout->addStretch();
}

void GeneratorWidget::SetupQuickModeUi(QGroupBox* group) {
    auto* layout = new QVBoxLayout(group);
    layout->setSpacing(8);

    // Use grid layout for alignment
    auto* grid = new QGridLayout();
    grid->setSpacing(8);

    // Difficulty row
    auto* diff_label = new QLabel(QString::fromUtf8("\u96be\u5ea6:"));
    grid->addWidget(diff_label, 0, 0, Qt::AlignRight);

    difficulty_button_group_ = new QButtonGroup(this);
    easy_radio_ = new QRadioButton(QString::fromUtf8("\u7b80\u5355"));
    medium_radio_ = new QRadioButton(QString::fromUtf8("\u4e2d\u7b49"));
    hard_radio_ = new QRadioButton(QString::fromUtf8("\u56f0\u96be"));
    expert_radio_ = new QRadioButton(QString::fromUtf8("\u4e13\u5bb6"));

    difficulty_button_group_->addButton(easy_radio_, 0);
    difficulty_button_group_->addButton(medium_radio_, 1);
    difficulty_button_group_->addButton(hard_radio_, 2);
    difficulty_button_group_->addButton(expert_radio_, 3);
    medium_radio_->setChecked(true);

    grid->addWidget(easy_radio_, 0, 1);
    grid->addWidget(medium_radio_, 0, 2);
    grid->addWidget(hard_radio_, 0, 3);
    grid->addWidget(expert_radio_, 0, 4);

    // Scale row
    auto* scale_label = new QLabel(QString::fromUtf8("\u89c4\u6a21:"));
    grid->addWidget(scale_label, 1, 0, Qt::AlignRight);

    scale_button_group_ = new QButtonGroup(this);
    small_radio_ = new QRadioButton(QString::fromUtf8("\u5c0f\u578b"));
    medium_scale_radio_ = new QRadioButton(QString::fromUtf8("\u4e2d\u578b"));
    large_radio_ = new QRadioButton(QString::fromUtf8("\u5927\u578b"));

    scale_button_group_->addButton(small_radio_, 0);
    scale_button_group_->addButton(medium_scale_radio_, 1);
    scale_button_group_->addButton(large_radio_, 2);
    medium_scale_radio_->setChecked(true);

    grid->addWidget(small_radio_, 1, 1);
    grid->addWidget(medium_scale_radio_, 1, 2);
    grid->addWidget(large_radio_, 1, 3);

    layout->addLayout(grid);
}

void GeneratorWidget::SetupManualModeUi(QGroupBox* group) {
    auto* layout = new QGridLayout(group);
    layout->setSpacing(4);
    layout->setColumnStretch(2, 1);  // Input column stretches

    int row = 0;

    // Problem scale row
    auto* scale_layout = new QHBoxLayout();
    n_spin_ = new QSpinBox();
    n_spin_->setRange(10, 2000);
    n_spin_->setValue(100);
    scale_layout->addWidget(new QLabel(QString::fromUtf8("\u8ba2\u5355\u6570:")));
    scale_layout->addWidget(n_spin_);

    t_spin_ = new QSpinBox();
    t_spin_->setRange(5, 90);
    t_spin_->setValue(30);
    scale_layout->addWidget(new QLabel(QString::fromUtf8("\u5468\u671f\u6570:")));
    scale_layout->addWidget(t_spin_);

    f_spin_ = new QSpinBox();
    f_spin_->setRange(2, 15);
    f_spin_->setValue(5);
    scale_layout->addWidget(new QLabel(QString::fromUtf8("\u6d41\u5411\u6570:")));
    scale_layout->addWidget(f_spin_);

    g_spin_ = new QSpinBox();
    g_spin_->setRange(2, 15);
    g_spin_->setValue(5);
    scale_layout->addWidget(new QLabel(QString::fromUtf8("\u7ec4\u522b\u6570:")));
    scale_layout->addWidget(g_spin_);

    layout->addWidget(new QLabel(QString::fromUtf8("\u89c4\u6a21")), row, 0, Qt::AlignRight);
    layout->addLayout(scale_layout, row, 1, 1, 2);
    row++;

    // Capacity utilization (产能利用比例)
    layout->addWidget(new QLabel(QString::fromUtf8("\u4ea7\u80fd\u5229\u7528\u6bd4\u4f8b")), row, 0, Qt::AlignRight);
    layout->addWidget(CreateHelpButton(
        QString::fromUtf8("\u751f\u4ea7\u8d44\u6e90\u7684\u5229\u7528\u7a0b\u5ea6"),
        QString::fromUtf8("\u4ea7\u80fd\u5229\u7528\u6bd4\u4f8b (Capacity Utilization)"),
        QString::fromUtf8(
            "\u82f1\u6587\u540d: capacity_utilization\n\n"
            "\u5b9a\u4e49: \u751f\u4ea7\u8d44\u6e90\u7684\u5229\u7528\u7a0b\u5ea6\uff0c\u5373\u603b\u9700\u6c42\u91cf/\u603b\u53ef\u7528\u4ea7\u80fd\u3002\n\n"
            "\u8ba1\u7b97\u516c\u5f0f:\n"
            "  Available = MachineCapacity * T * utilization - SetupOverhead\n"
            "  TotalDemand = Available\n\n"
            "\u53d6\u503c\u8303\u56f4: 0.30 ~ 0.99\n\n"
            "\u96be\u5ea6\u5f71\u54cd: \u6700\u5173\u952e\u56e0\u7d20(\u6743\u91cd~70%)\u3002\n"
            "  - 0.50: \u6781\u6613 (Gap<1%)\n"
            "  - 0.70: \u4e2d\u7b49 (Gap 2-8%)\n"
            "  - 0.85: \u56f0\u96be (Gap 5-20%)\n"
            "  - 0.95: \u6781\u96be (Gap 15-50%+)\n\n"
            "\u63a8\u8350\u503c: 0.70 (\u4e2d\u7b49\u96be\u5ea6)")), row, 1);
    capacity_spin_ = new QDoubleSpinBox();
    capacity_spin_->setRange(0.30, 0.99);
    capacity_spin_->setSingleStep(0.05);
    capacity_spin_->setValue(0.70);
    capacity_spin_->setDecimals(2);
    layout->addWidget(capacity_spin_, row, 2);
    row++;

    // Time window offset (时窗偏移期数)
    layout->addWidget(new QLabel(QString::fromUtf8("\u65f6\u7a97\u504f\u79fb\u671f\u6570")), row, 0, Qt::AlignRight);
    layout->addWidget(CreateHelpButton(
        QString::fromUtf8("\u8ba2\u5355\u65f6\u95f4\u7a97\u7684\u534a\u5bbd"),
        QString::fromUtf8("\u65f6\u7a97\u504f\u79fb\u671f\u6570 (Time Window Offset)"),
        QString::fromUtf8(
            "\u82f1\u6587\u540d: time_window_offset\n\n"
            "\u5b9a\u4e49: \u8ba2\u5355\u65f6\u95f4\u7a97\u7684\u534a\u5bbd\uff0c\u7a97\u53e3\u5927\u5c0f = 2*offset + 1\u3002\n\n"
            "\u751f\u6210\u65b9\u5f0f:\n"
            "  ew[i] = max(1, due_period - offset)\n"
            "  lw[i] = min(T, due_period + offset)\n\n"
            "\u53d6\u503c\u8303\u56f4: 1 ~ 15\n\n"
            "\u96be\u5ea6\u5f71\u54cd: \u6743\u91cd~15%\n"
            "  - offset=3: \u7a97\u53e3=7\u671f, \u56f0\u96be\n"
            "  - offset=5: \u7a97\u53e3=11\u671f, \u4e2d\u7b49\n"
            "  - offset=10: \u7a97\u53e3=21\u671f, \u7b80\u5355\n\n"
            "\u63a8\u8350\u503c: 5 (\u4e2d\u7b49\u96be\u5ea6)")), row, 1);
    offset_spin_ = new QSpinBox();
    offset_spin_->setRange(1, 15);
    offset_spin_->setValue(5);
    layout->addWidget(offset_spin_, row, 2);
    row++;

    // Demand CV (需求变异系数)
    layout->addWidget(new QLabel(QString::fromUtf8("\u9700\u6c42\u53d8\u5f02\u7cfb\u6570")), row, 0, Qt::AlignRight);
    layout->addWidget(CreateHelpButton(
        QString::fromUtf8("\u9700\u6c42\u5206\u5e03\u7684\u79bb\u6563\u7a0b\u5ea6"),
        QString::fromUtf8("\u9700\u6c42\u53d8\u5f02\u7cfb\u6570 (Demand CV)"),
        QString::fromUtf8(
            "\u82f1\u6587\u540d: demand_cv (Coefficient of Variation)\n\n"
            "\u5b9a\u4e49: \u9700\u6c42\u5206\u5e03\u7684\u79bb\u6563\u7a0b\u5ea6 = \u6807\u51c6\u5dee/\u5747\u503c\u3002\n\n"
            "\u751f\u6210\u65b9\u5f0f:\n"
            "  demand = base * (1 + N(0, cv))\n"
            "  \u5176\u4e2d N(0,cv) \u4e3a\u6b63\u6001\u5206\u5e03\u968f\u673a\u6570\n\n"
            "\u53d6\u503c\u8303\u56f4: 0.0 ~ 0.60\n\n"
            "\u96be\u5ea6\u5f71\u54cd:\n"
            "  - 0.10: \u9700\u6c42\u5e73\u7a33, \u7b80\u5355\n"
            "  - 0.25: \u4e2d\u7b49\u6ce2\u52a8, \u4e2d\u7b49\n"
            "  - 0.40+: \u9ad8\u5ea6\u6ce2\u52a8, \u56f0\u96be\n\n"
            "\u63a8\u8350\u503c: 0.25 (\u4e2d\u7b49\u6ce2\u52a8)")), row, 1);
    demand_cv_spin_ = new QDoubleSpinBox();
    demand_cv_spin_->setRange(0.0, 0.60);
    demand_cv_spin_->setSingleStep(0.05);
    demand_cv_spin_->setValue(0.25);
    demand_cv_spin_->setDecimals(2);
    layout->addWidget(demand_cv_spin_, row, 2);
    row++;

    // Peak ratio (需求高峰比例)
    layout->addWidget(new QLabel(QString::fromUtf8("\u9700\u6c42\u9ad8\u5cf0\u6bd4\u4f8b")), row, 0, Qt::AlignRight);
    layout->addWidget(CreateHelpButton(
        QString::fromUtf8("\u968f\u673a\u53d8\u4e3a\u9ad8\u5cf0\u671f\u7684\u6982\u7387"),
        QString::fromUtf8("\u9700\u6c42\u9ad8\u5cf0\u6bd4\u4f8b (Peak Ratio)"),
        QString::fromUtf8(
            "\u82f1\u6587\u540d: peak_ratio\n\n"
            "\u5b9a\u4e49: \u67d0\u65f6\u6bb5\u968f\u673a\u53d8\u4e3a\u9700\u6c42\u9ad8\u5cf0\u7684\u6982\u7387\u3002\n\n"
            "\u751f\u6210\u65b9\u5f0f:\n"
            "  if (rand() < peak_ratio):\n"
            "    demand *= peak_multiplier\n\n"
            "\u53d6\u503c\u8303\u56f4: 0.0 ~ 0.40\n\n"
            "\u96be\u5ea6\u5f71\u54cd:\n"
            "  - 0.0: \u65e0\u9ad8\u5cf0, \u7b80\u5355\n"
            "  - 0.15: \u5076\u5c14\u9ad8\u5cf0, \u4e2d\u7b49\n"
            "  - 0.30+: \u9891\u7e41\u9ad8\u5cf0, \u56f0\u96be\n\n"
            "\u63a8\u8350\u503c: 0.15")), row, 1);
    peak_ratio_spin_ = new QDoubleSpinBox();
    peak_ratio_spin_->setRange(0.0, 0.40);
    peak_ratio_spin_->setSingleStep(0.05);
    peak_ratio_spin_->setValue(0.15);
    peak_ratio_spin_->setDecimals(2);
    layout->addWidget(peak_ratio_spin_, row, 2);
    row++;

    // Peak multiplier (需求高峰倍数)
    layout->addWidget(new QLabel(QString::fromUtf8("\u9700\u6c42\u9ad8\u5cf0\u500d\u6570")), row, 0, Qt::AlignRight);
    layout->addWidget(CreateHelpButton(
        QString::fromUtf8("\u9ad8\u5cf0\u671f\u9700\u6c42\u653e\u5927\u500d\u6570"),
        QString::fromUtf8("\u9700\u6c42\u9ad8\u5cf0\u500d\u6570 (Peak Multiplier)"),
        QString::fromUtf8(
            "\u82f1\u6587\u540d: peak_multiplier\n\n"
            "\u5b9a\u4e49: \u9ad8\u5cf0\u671f\u9700\u6c42\u7684\u653e\u5927\u500d\u6570\u3002\n\n"
            "\u751f\u6210\u65b9\u5f0f:\n"
            "  peak_demand = base_demand * peak_multiplier\n\n"
            "\u53d6\u503c\u8303\u56f4: 1.0 ~ 4.0\n\n"
            "\u96be\u5ea6\u5f71\u54cd:\n"
            "  - 1.5: \u5c0f\u5e45\u9ad8\u5cf0, \u7b80\u5355\n"
            "  - 2.0: \u4e2d\u7b49\u9ad8\u5cf0, \u4e2d\u7b49\n"
            "  - 3.0+: \u5267\u70c8\u9ad8\u5cf0, \u56f0\u96be\n\n"
            "\u63a8\u8350\u503c: 2.0")), row, 1);
    peak_mult_spin_ = new QDoubleSpinBox();
    peak_mult_spin_->setRange(1.0, 4.0);
    peak_mult_spin_->setSingleStep(0.25);
    peak_mult_spin_->setValue(2.0);
    peak_mult_spin_->setDecimals(2);
    layout->addWidget(peak_mult_spin_, row, 2);
    row++;

    // Urgent ratio (紧急订单比例)
    layout->addWidget(new QLabel(QString::fromUtf8("\u7d27\u6025\u8ba2\u5355\u6bd4\u4f8b")), row, 0, Qt::AlignRight);
    layout->addWidget(CreateHelpButton(
        QString::fromUtf8("\u7a84\u65f6\u95f4\u7a97\u7684\u8ba2\u5355\u6bd4\u4f8b"),
        QString::fromUtf8("\u7d27\u6025\u8ba2\u5355\u6bd4\u4f8b (Urgent Ratio)"),
        QString::fromUtf8(
            "\u82f1\u6587\u540d: urgent_ratio\n\n"
            "\u5b9a\u4e49: \u91c7\u7528\u7a84\u65f6\u95f4\u7a97(offset/2)\u7684\u8ba2\u5355\u6bd4\u4f8b\u3002\n\n"
            "\u751f\u6210\u65b9\u5f0f:\n"
            "  if (rand() < urgent_ratio):\n"
            "    order.offset = base_offset / 2  // \u66f4\u7d27\u7684\u65f6\u95f4\u7a97\n\n"
            "\u53d6\u503c\u8303\u56f4: 0.0 ~ 0.40\n\n"
            "\u96be\u5ea6\u5f71\u54cd:\n"
            "  - 0.0: \u65e0\u7d27\u6025\u8ba2\u5355, \u7b80\u5355\n"
            "  - 0.10: \u5c11\u91cf\u7d27\u6025, \u4e2d\u7b49\n"
            "  - 0.25+: \u5927\u91cf\u7d27\u6025, \u56f0\u96be\n\n"
            "\u63a8\u8350\u503c: 0.10")), row, 1);
    urgent_spin_ = new QDoubleSpinBox();
    urgent_spin_->setRange(0.0, 0.40);
    urgent_spin_->setSingleStep(0.05);
    urgent_spin_->setValue(0.10);
    urgent_spin_->setDecimals(2);
    layout->addWidget(urgent_spin_, row, 2);
    row++;

    // Flexible ratio (灵活订单比例)
    layout->addWidget(new QLabel(QString::fromUtf8("\u7075\u6d3b\u8ba2\u5355\u6bd4\u4f8b")), row, 0, Qt::AlignRight);
    layout->addWidget(CreateHelpButton(
        QString::fromUtf8("\u5bbd\u65f6\u95f4\u7a97\u7684\u8ba2\u5355\u6bd4\u4f8b"),
        QString::fromUtf8("\u7075\u6d3b\u8ba2\u5355\u6bd4\u4f8b (Flexible Ratio)"),
        QString::fromUtf8(
            "\u82f1\u6587\u540d: flexible_ratio\n\n"
            "\u5b9a\u4e49: \u91c7\u7528\u5bbd\u65f6\u95f4\u7a97(offset*2)\u7684\u8ba2\u5355\u6bd4\u4f8b\u3002\n\n"
            "\u751f\u6210\u65b9\u5f0f:\n"
            "  if (rand() < flexible_ratio):\n"
            "    order.offset = min(base_offset * 2, T/3)  // \u66f4\u5bbd\u7684\u65f6\u95f4\u7a97\n\n"
            "\u53d6\u503c\u8303\u56f4: 0.0 ~ 0.40\n\n"
            "\u96be\u5ea6\u5f71\u54cd:\n"
            "  - 0.0: \u65e0\u7075\u6d3b\u8ba2\u5355, \u7a0d\u96be\n"
            "  - 0.20: \u90e8\u5206\u7075\u6d3b, \u4e2d\u7b49\n"
            "  - 0.40: \u5927\u91cf\u7075\u6d3b, \u7b80\u5355\n\n"
            "\u63a8\u8350\u503c: 0.20")), row, 1);
    flexible_spin_ = new QDoubleSpinBox();
    flexible_spin_->setRange(0.0, 0.40);
    flexible_spin_->setSingleStep(0.05);
    flexible_spin_->setValue(0.20);
    flexible_spin_->setDecimals(2);
    layout->addWidget(flexible_spin_, row, 2);
    row++;

    // Zoom (容量缩放因子)
    layout->addWidget(new QLabel(QString::fromUtf8("\u5bb9\u91cf\u7f29\u653e\u56e0\u5b50")), row, 0, Qt::AlignRight);
    layout->addWidget(CreateHelpButton(
        QString::fromUtf8("\u673a\u5668\u4ea7\u80fd\u7684\u57fa\u7840\u7f29\u653e\u7cfb\u6570"),
        QString::fromUtf8("\u5bb9\u91cf\u7f29\u653e\u56e0\u5b50 (Zoom)"),
        QString::fromUtf8(
            "\u82f1\u6587\u540d: zoom\n\n"
            "\u5b9a\u4e49: \u673a\u5668\u4ea7\u80fd\u7684\u57fa\u7840\u7f29\u653e\u7cfb\u6570\u3002\n\n"
            "\u8ba1\u7b97\u516c\u5f0f:\n"
            "  MachineCapacity = zoom * T\n"
            "  TotalCapacity = MachineCapacity * T = zoom * T^2\n\n"
            "\u53d6\u503c\u8303\u56f4: 30 ~ 100\n\n"
            "\u96be\u5ea6\u5f71\u54cd:\n"
            "  - zoom\u5927: \u4ea7\u80fd\u7edd\u5bf9\u503c\u5927, \u7b80\u5355\n"
            "  - zoom\u5c0f: \u4ea7\u80fd\u7edd\u5bf9\u503c\u5c0f, \u8f83\u96be\n\n"
            "\u63a8\u8350\u503c: 60")), row, 1);
    zoom_spin_ = new QSpinBox();
    zoom_spin_->setRange(30, 100);
    zoom_spin_->setValue(60);
    layout->addWidget(zoom_spin_, row, 2);
    row++;

    // Cost correlation checkbox (切换产能关联) - at bottom
    layout->addWidget(new QLabel(QString::fromUtf8("\u5207\u6362\u4ea7\u80fd\u5173\u8054")), row, 0, Qt::AlignRight);
    layout->addWidget(CreateHelpButton(
        QString::fromUtf8("Setup\u6210\u672c\u4e0e\u8d44\u6e90\u6d88\u8017\u6b63\u76f8\u5173"),
        QString::fromUtf8("\u5207\u6362\u4ea7\u80fd\u5173\u8054 (Cost Correlation)"),
        QString::fromUtf8(
            "\u82f1\u6587\u540d: cost_correlation\n\n"
            "\u5b9a\u4e49: \u542f\u52a8(Setup)\u6210\u672c\u4e0e\u8d44\u6e90\u6d88\u8017\u662f\u5426\u6b63\u76f8\u5173\u3002\n\n"
            "\u751f\u6210\u65b9\u5f0f:\n"
            "  \u542f\u7528\u65f6:\n"
            "    cost_Y = base_cost * factor\n"
            "    usage_Y = base_usage * (0.5 + 0.5*factor)\n"
            "  \u7981\u7528\u65f6:\n"
            "    cost_Y \u548c usage_Y \u72ec\u7acb\u968f\u673a\u751f\u6210\n\n"
            "\u53d6\u503c: \u5f00/\u5173\n\n"
            "\u96be\u5ea6\u5f71\u54cd:\n"
            "  - \u542f\u7528: \u66f4\u771f\u5b9e\u7684\u751f\u4ea7\u573a\u666f\n"
            "  - \u7981\u7528: \u66f4\u968f\u673a\u7684\u6210\u672c\u7ed3\u6784\n\n"
            "\u63a8\u8350\u503c: \u542f\u7528")), row, 1);
    cost_corr_check_ = new QCheckBox();
    cost_corr_check_->setChecked(true);
    layout->addWidget(cost_corr_check_, row, 2);
}

void GeneratorWidget::SetupConnections() {
    connect(mode_combo_, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &GeneratorWidget::OnModeChanged);

    connect(difficulty_button_group_, &QButtonGroup::idClicked,
            this, &GeneratorWidget::OnDifficultyChanged);
    connect(scale_button_group_, &QButtonGroup::idClicked,
            this, &GeneratorWidget::OnScaleChanged);

    connect(browse_button_, &QPushButton::clicked,
            this, &GeneratorWidget::OnBrowseOutput);
    connect(generate_button_, &QPushButton::clicked,
            this, &GeneratorWidget::OnGenerateClicked);

    // Manual mode value changes
    connect(n_spin_, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &GeneratorWidget::UpdatePreview);
    connect(t_spin_, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &GeneratorWidget::UpdatePreview);
    connect(capacity_spin_, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &GeneratorWidget::UpdatePreview);
    connect(offset_spin_, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &GeneratorWidget::UpdatePreview);
    connect(demand_cv_spin_, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &GeneratorWidget::UpdatePreview);
}

void GeneratorWidget::OnModeChanged() {
    bool quick = (mode_combo_->currentIndex() == 0);
    quick_group_->setVisible(quick);
    manual_group_->setVisible(!quick);
    UpdatePreview();
}

void GeneratorWidget::OnDifficultyChanged() {
    UpdatePreview();
}

void GeneratorWidget::OnScaleChanged() {
    UpdatePreview();
}

void GeneratorWidget::OnBrowseOutput() {
    QString path = QFileDialog::getExistingDirectory(this,
        QString::fromUtf8("\u9009\u62e9\u8f93\u51fa\u76ee\u5f55"),
        output_edit_->text());

    if (!path.isEmpty()) {
        if (!path.endsWith('/') && !path.endsWith('\\')) {
            path += '/';
        }
        output_edit_->setText(path);
    }
}

void GeneratorWidget::OnGenerateClicked() {
    GeneratorConfig config = GetConfig();
    emit GenerateRequested(config);
}

void GeneratorWidget::UpdatePreview() {
    GeneratorConfig config = GetConfig();
    double score = DifficultyMapper::EstimateDifficultyScore(config);
    QString gap = DifficultyMapper::EstimateGap(config);

    QString preview = QString::fromUtf8(
        "\u8ba2\u5355\u6570=%1  \u5468\u671f\u6570=%2  \u6d41\u5411\u6570=%3  \u7ec4\u522b\u6570=%4\n"
        "\u4ea7\u80fd\u5229\u7528\u7387 = %5\n"
        "\u65f6\u95f4\u7a97\u504f\u79fb = %6\n"
        "\u9700\u6c42\u53d8\u5f02\u7cfb\u6570 = %7\n"
        "\u96be\u5ea6\u8bc4\u5206 = %8\n"
        "\u9884\u4f30Gap = %9")
        .arg(config.N)
        .arg(config.T)
        .arg(config.F)
        .arg(config.G)
        .arg(config.capacity_utilization, 0, 'f', 2)
        .arg(config.time_window_offset)
        .arg(config.demand_cv, 0, 'f', 2)
        .arg(score, 0, 'f', 2)
        .arg(gap);

    preview_label_->setText(preview);
    emit ConfigChanged();
}

GeneratorConfig GeneratorWidget::GetConfig() const {
    GeneratorConfig config;

    if (mode_combo_->currentIndex() == 0) {
        // Quick mode - use presets
        int diff_id = difficulty_button_group_->checkedId();
        int scale_id = scale_button_group_->checkedId();

        DifficultyLevel diff = static_cast<DifficultyLevel>(diff_id);
        ScaleLevel scale = static_cast<ScaleLevel>(scale_id);

        config = DifficultyMapper::GetPreset(diff, scale);
    } else {
        // Manual mode
        config.N = n_spin_->value();
        config.T = t_spin_->value();
        config.F = f_spin_->value();
        config.G = g_spin_->value();
        config.capacity_utilization = capacity_spin_->value();
        config.time_window_offset = offset_spin_->value();
        config.demand_cv = demand_cv_spin_->value();
        config.peak_ratio = peak_ratio_spin_->value();
        config.peak_multiplier = peak_mult_spin_->value();
        config.urgent_ratio = urgent_spin_->value();
        config.flexible_ratio = flexible_spin_->value();
        config.cost_correlation = cost_corr_check_->isChecked();
        config.zoom = zoom_spin_->value();
    }

    // Common settings
    config.seed = seed_spin_->value();
    config.count = count_spin_->value();
    config.output_path = output_edit_->text();

    return config;
}

void GeneratorWidget::SetOutputPath(const QString& path) {
    output_edit_->setText(path);
}

void GeneratorWidget::ApplyPreset(const GeneratorConfig& config) {
    n_spin_->setValue(config.N);
    t_spin_->setValue(config.T);
    f_spin_->setValue(config.F);
    g_spin_->setValue(config.G);
    capacity_spin_->setValue(config.capacity_utilization);
    offset_spin_->setValue(config.time_window_offset);
    demand_cv_spin_->setValue(config.demand_cv);
    peak_ratio_spin_->setValue(config.peak_ratio);
    peak_mult_spin_->setValue(config.peak_multiplier);
    urgent_spin_->setValue(config.urgent_ratio);
    flexible_spin_->setValue(config.flexible_ratio);
    cost_corr_check_->setChecked(config.cost_correlation);
    zoom_spin_->setValue(config.zoom);
}

QToolButton* GeneratorWidget::CreateHelpButton(const QString& tooltip,
                                                const QString& detail_title,
                                                const QString& detail_content) {
    auto* button = new QToolButton(this);
    button->setText("?");
    button->setFixedSize(18, 18);
    button->setToolTip(tooltip);
    button->setStyleSheet(
        "QToolButton { border: 1px solid #999; border-radius: 9px; "
        "background: #f0f0f0; font-weight: bold; font-size: 10px; }"
        "QToolButton:hover { background: #e0e0e0; }");

    // Store detail info as dynamic properties
    button->setProperty("detail_title", detail_title);
    button->setProperty("detail_content", detail_content);

    connect(button, &QToolButton::clicked, this, &GeneratorWidget::OnHelpButtonClicked);

    return button;
}

void GeneratorWidget::OnHelpButtonClicked() {
    auto* button = qobject_cast<QToolButton*>(sender());
    if (!button) return;

    QString title = button->property("detail_title").toString();
    QString content = button->property("detail_content").toString();

    QMessageBox::information(this, title, content);
}
