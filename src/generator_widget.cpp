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
    mode_layout->addWidget(new QLabel("Mode:"));
    mode_combo_ = new QComboBox();
    mode_combo_->addItem("Quick");
    mode_combo_->addItem("Manual");
    mode_layout->addWidget(mode_combo_);
    mode_layout->addStretch();
    main_layout->addLayout(mode_layout);

    // Quick mode group
    quick_group_ = new QGroupBox("Quick Settings");
    SetupQuickModeUi(quick_group_);
    main_layout->addWidget(quick_group_);

    // Manual mode group
    manual_group_ = new QGroupBox("Manual Settings");
    SetupManualModeUi(manual_group_);
    manual_group_->setVisible(false);
    main_layout->addWidget(manual_group_);

    // Common settings group
    auto* common_group = new QGroupBox("Output");
    auto* common_layout = new QFormLayout(common_group);
    common_layout->setSpacing(4);

    seed_spin_ = new QSpinBox();
    seed_spin_->setRange(0, 999999);
    seed_spin_->setValue(0);
    seed_spin_->setSpecialValueText("Auto");
    common_layout->addRow("Seed:", seed_spin_);

    count_spin_ = new QSpinBox();
    count_spin_->setRange(1, 100);
    count_spin_->setValue(1);
    common_layout->addRow("Count:", count_spin_);

    auto* path_layout = new QHBoxLayout();
    output_edit_ = new QLineEdit();
    output_edit_->setText("D:/YM-Code/LS-NTGF-Data-Cap/data/");
    browse_button_ = new QPushButton("...");
    browse_button_->setFixedWidth(32);
    path_layout->addWidget(output_edit_);
    path_layout->addWidget(browse_button_);
    common_layout->addRow("Path:", path_layout);

    main_layout->addWidget(common_group);

    // Preview label
    preview_label_ = new QLabel();
    preview_label_->setStyleSheet("background: #f0f0f0; padding: 8px; font-family: monospace; font-size: 9pt;");
    preview_label_->setWordWrap(true);
    main_layout->addWidget(preview_label_);

    // Generate button
    generate_button_ = new QPushButton("Generate");
    generate_button_->setMinimumHeight(32);
    generate_button_->setStyleSheet("font-weight: bold;");
    main_layout->addWidget(generate_button_);

    main_layout->addStretch();
}

void GeneratorWidget::SetupQuickModeUi(QGroupBox* group) {
    auto* layout = new QVBoxLayout(group);
    layout->setSpacing(8);

    // Difficulty selection
    auto* diff_label = new QLabel("Difficulty:");
    layout->addWidget(diff_label);

    auto* diff_layout = new QHBoxLayout();
    difficulty_button_group_ = new QButtonGroup(this);

    easy_radio_ = new QRadioButton("Easy");
    medium_radio_ = new QRadioButton("Medium");
    hard_radio_ = new QRadioButton("Hard");
    expert_radio_ = new QRadioButton("Expert");

    difficulty_button_group_->addButton(easy_radio_, 0);
    difficulty_button_group_->addButton(medium_radio_, 1);
    difficulty_button_group_->addButton(hard_radio_, 2);
    difficulty_button_group_->addButton(expert_radio_, 3);

    medium_radio_->setChecked(true);

    diff_layout->addWidget(easy_radio_);
    diff_layout->addWidget(medium_radio_);
    diff_layout->addWidget(hard_radio_);
    diff_layout->addWidget(expert_radio_);
    layout->addLayout(diff_layout);

    // Scale selection
    auto* scale_label = new QLabel("Scale:");
    layout->addWidget(scale_label);

    auto* scale_layout = new QHBoxLayout();
    scale_button_group_ = new QButtonGroup(this);

    small_radio_ = new QRadioButton("Small");
    medium_scale_radio_ = new QRadioButton("Medium");
    large_radio_ = new QRadioButton("Large");

    scale_button_group_->addButton(small_radio_, 0);
    scale_button_group_->addButton(medium_scale_radio_, 1);
    scale_button_group_->addButton(large_radio_, 2);

    medium_scale_radio_->setChecked(true);

    scale_layout->addWidget(small_radio_);
    scale_layout->addWidget(medium_scale_radio_);
    scale_layout->addWidget(large_radio_);
    layout->addLayout(scale_layout);
}

void GeneratorWidget::SetupManualModeUi(QGroupBox* group) {
    auto* layout = new QFormLayout(group);
    layout->setSpacing(4);

    // Problem scale
    auto* scale_layout = new QHBoxLayout();
    n_spin_ = new QSpinBox();
    n_spin_->setRange(10, 2000);
    n_spin_->setValue(100);
    scale_layout->addWidget(new QLabel("N:"));
    scale_layout->addWidget(n_spin_);

    t_spin_ = new QSpinBox();
    t_spin_->setRange(5, 90);
    t_spin_->setValue(30);
    scale_layout->addWidget(new QLabel("T:"));
    scale_layout->addWidget(t_spin_);

    f_spin_ = new QSpinBox();
    f_spin_->setRange(2, 15);
    f_spin_->setValue(5);
    scale_layout->addWidget(new QLabel("F:"));
    scale_layout->addWidget(f_spin_);

    g_spin_ = new QSpinBox();
    g_spin_->setRange(2, 15);
    g_spin_->setValue(5);
    scale_layout->addWidget(new QLabel("G:"));
    scale_layout->addWidget(g_spin_);

    layout->addRow("Scale:", scale_layout);

    // Capacity utilization
    capacity_spin_ = new QDoubleSpinBox();
    capacity_spin_->setRange(0.30, 0.99);
    capacity_spin_->setSingleStep(0.05);
    capacity_spin_->setValue(0.70);
    capacity_spin_->setDecimals(2);
    layout->addRow("Capacity Util:", capacity_spin_);

    // Time window offset
    offset_spin_ = new QSpinBox();
    offset_spin_->setRange(1, 15);
    offset_spin_->setValue(5);
    layout->addRow("Time Offset:", offset_spin_);

    // Demand CV
    demand_cv_spin_ = new QDoubleSpinBox();
    demand_cv_spin_->setRange(0.0, 0.60);
    demand_cv_spin_->setSingleStep(0.05);
    demand_cv_spin_->setValue(0.25);
    demand_cv_spin_->setDecimals(2);
    layout->addRow("Demand CV:", demand_cv_spin_);

    // Peak ratio
    peak_ratio_spin_ = new QDoubleSpinBox();
    peak_ratio_spin_->setRange(0.0, 0.40);
    peak_ratio_spin_->setSingleStep(0.05);
    peak_ratio_spin_->setValue(0.15);
    peak_ratio_spin_->setDecimals(2);
    layout->addRow("Peak Ratio:", peak_ratio_spin_);

    // Peak multiplier
    peak_mult_spin_ = new QDoubleSpinBox();
    peak_mult_spin_->setRange(1.0, 4.0);
    peak_mult_spin_->setSingleStep(0.25);
    peak_mult_spin_->setValue(2.0);
    peak_mult_spin_->setDecimals(2);
    layout->addRow("Peak Mult:", peak_mult_spin_);

    // Urgent ratio
    urgent_spin_ = new QDoubleSpinBox();
    urgent_spin_->setRange(0.0, 0.40);
    urgent_spin_->setSingleStep(0.05);
    urgent_spin_->setValue(0.10);
    urgent_spin_->setDecimals(2);
    layout->addRow("Urgent Ratio:", urgent_spin_);

    // Flexible ratio
    flexible_spin_ = new QDoubleSpinBox();
    flexible_spin_->setRange(0.0, 0.40);
    flexible_spin_->setSingleStep(0.05);
    flexible_spin_->setValue(0.20);
    flexible_spin_->setDecimals(2);
    layout->addRow("Flexible Ratio:", flexible_spin_);

    // Cost correlation
    cost_corr_check_ = new QCheckBox();
    cost_corr_check_->setChecked(true);
    layout->addRow("Cost Corr:", cost_corr_check_);

    // Zoom
    zoom_spin_ = new QSpinBox();
    zoom_spin_->setRange(30, 100);
    zoom_spin_->setValue(60);
    layout->addRow("Zoom:", zoom_spin_);
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
        "Select Output Directory",
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

    QString preview = QString(
        "N=%1  T=%2  F=%3  G=%4\n"
        "capacity_util = %5\n"
        "time_offset = %6\n"
        "demand_cv = %7\n"
        "difficulty_score = %8\n"
        "estimated_gap = %9")
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
