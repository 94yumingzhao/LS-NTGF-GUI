// generator_widget.cpp - Instance Generator Widget Implementation

#include "generator_widget.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QLabel>
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
#include <QFrame>

GeneratorWidget::GeneratorWidget(QWidget* parent)
    : QWidget(parent)
    , is_quick_mode_(true) {
    SetupUi();
    SetupConnections();
    UpdatePreview();
}

void GeneratorWidget::SetupUi() {
    auto* main_layout = new QVBoxLayout(this);
    main_layout->setSpacing(8);
    main_layout->setContentsMargins(4, 4, 4, 4);

    // Mode selector buttons
    auto* mode_layout = new QHBoxLayout();
    quick_mode_button_ = new QPushButton(QString::fromUtf8("快速模式"));
    manual_mode_button_ = new QPushButton(QString::fromUtf8("手动模式"));
    quick_mode_button_->setCheckable(true);
    manual_mode_button_->setCheckable(true);
    quick_mode_button_->setChecked(true);
    quick_mode_button_->setStyleSheet(
        "QPushButton { padding: 6px 12px; }"
        "QPushButton:checked { background-color: #228be6; color: white; font-weight: bold; }");
    manual_mode_button_->setStyleSheet(
        "QPushButton { padding: 6px 12px; }"
        "QPushButton:checked { background-color: #228be6; color: white; font-weight: bold; }");
    mode_layout->addWidget(quick_mode_button_);
    mode_layout->addWidget(manual_mode_button_);
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
    layout->setSpacing(6);
    layout->setColumnStretch(1, 0);
    layout->setColumnStretch(2, 1);
    layout->setColumnStretch(3, 0);
    layout->setColumnStretch(4, 1);

    int row = 0;

    // Problem scale - split into 2x2 grid for better spacing
    // Row 1: N and T
    layout->addWidget(new QLabel(QString::fromUtf8("订单数(N)")), row, 0, Qt::AlignRight);
    n_spin_ = new QSpinBox();
    n_spin_->setRange(10, 2000);
    n_spin_->setValue(100);
    n_spin_->setMinimumWidth(70);
    layout->addWidget(n_spin_, row, 1);

    layout->addWidget(new QLabel(QString::fromUtf8("周期数(T)")), row, 2, Qt::AlignRight);
    t_spin_ = new QSpinBox();
    t_spin_->setRange(5, 90);
    t_spin_->setValue(30);
    t_spin_->setMinimumWidth(70);
    layout->addWidget(t_spin_, row, 3);
    row++;

    // Row 2: F and G
    layout->addWidget(new QLabel(QString::fromUtf8("流向数(F)")), row, 0, Qt::AlignRight);
    f_spin_ = new QSpinBox();
    f_spin_->setRange(2, 15);
    f_spin_->setValue(5);
    f_spin_->setMinimumWidth(70);
    layout->addWidget(f_spin_, row, 1);

    layout->addWidget(new QLabel(QString::fromUtf8("组别数(G)")), row, 2, Qt::AlignRight);
    g_spin_ = new QSpinBox();
    g_spin_->setRange(2, 15);
    g_spin_->setValue(5);
    g_spin_->setMinimumWidth(70);
    layout->addWidget(g_spin_, row, 3);
    row++;

    // Separator line after scale rows
    auto* separator = new QFrame();
    separator->setFrameShape(QFrame::HLine);
    separator->setFrameShadow(QFrame::Sunken);
    layout->addWidget(separator, row, 0, 1, 4);
    row++;

    // Capacity utilization and Time window offset (same row)
    layout->addWidget(new QLabel(QString::fromUtf8("产能利用")), row, 0, Qt::AlignRight);
    capacity_spin_ = new QDoubleSpinBox();
    capacity_spin_->setRange(0.30, 0.99);
    capacity_spin_->setSingleStep(0.05);
    capacity_spin_->setValue(0.70);
    capacity_spin_->setDecimals(2);
    capacity_spin_->setMinimumWidth(70);
    layout->addWidget(capacity_spin_, row, 1);

    layout->addWidget(new QLabel(QString::fromUtf8("时窗偏移")), row, 2, Qt::AlignRight);
    offset_spin_ = new QSpinBox();
    offset_spin_->setRange(1, 15);
    offset_spin_->setValue(5);
    offset_spin_->setMinimumWidth(70);
    layout->addWidget(offset_spin_, row, 3);
    row++;

    // Demand CV and Zoom (same row)
    layout->addWidget(new QLabel(QString::fromUtf8("需求变异")), row, 0, Qt::AlignRight);
    demand_cv_spin_ = new QDoubleSpinBox();
    demand_cv_spin_->setRange(0.0, 0.60);
    demand_cv_spin_->setSingleStep(0.05);
    demand_cv_spin_->setValue(0.25);
    demand_cv_spin_->setDecimals(2);
    demand_cv_spin_->setMinimumWidth(70);
    layout->addWidget(demand_cv_spin_, row, 1);

    layout->addWidget(new QLabel(QString::fromUtf8("容量缩放")), row, 2, Qt::AlignRight);
    zoom_spin_ = new QSpinBox();
    zoom_spin_->setRange(30, 100);
    zoom_spin_->setValue(60);
    zoom_spin_->setMinimumWidth(70);
    layout->addWidget(zoom_spin_, row, 3);
    row++;

    // Peak ratio and Peak multiplier (same row)
    layout->addWidget(new QLabel(QString::fromUtf8("高峰比例")), row, 0, Qt::AlignRight);
    peak_ratio_spin_ = new QDoubleSpinBox();
    peak_ratio_spin_->setRange(0.0, 0.40);
    peak_ratio_spin_->setSingleStep(0.05);
    peak_ratio_spin_->setValue(0.15);
    peak_ratio_spin_->setDecimals(2);
    peak_ratio_spin_->setMinimumWidth(70);
    layout->addWidget(peak_ratio_spin_, row, 1);

    layout->addWidget(new QLabel(QString::fromUtf8("高峰倍数")), row, 2, Qt::AlignRight);
    peak_mult_spin_ = new QDoubleSpinBox();
    peak_mult_spin_->setRange(1.0, 4.0);
    peak_mult_spin_->setSingleStep(0.25);
    peak_mult_spin_->setValue(2.0);
    peak_mult_spin_->setDecimals(2);
    peak_mult_spin_->setMinimumWidth(70);
    layout->addWidget(peak_mult_spin_, row, 3);
    row++;

    // Urgent ratio and Flexible ratio (same row)
    layout->addWidget(new QLabel(QString::fromUtf8("紧急比例")), row, 0, Qt::AlignRight);
    urgent_spin_ = new QDoubleSpinBox();
    urgent_spin_->setRange(0.0, 0.40);
    urgent_spin_->setSingleStep(0.05);
    urgent_spin_->setValue(0.10);
    urgent_spin_->setDecimals(2);
    urgent_spin_->setMinimumWidth(70);
    layout->addWidget(urgent_spin_, row, 1);

    layout->addWidget(new QLabel(QString::fromUtf8("灵活比例")), row, 2, Qt::AlignRight);
    flexible_spin_ = new QDoubleSpinBox();
    flexible_spin_->setRange(0.0, 0.40);
    flexible_spin_->setSingleStep(0.05);
    flexible_spin_->setValue(0.20);
    flexible_spin_->setDecimals(2);
    flexible_spin_->setMinimumWidth(70);
    layout->addWidget(flexible_spin_, row, 3);
    row++;

    // Cost correlation checkbox
    layout->addWidget(new QLabel(QString::fromUtf8("成本关联")), row, 0, Qt::AlignRight);
    cost_corr_check_ = new QCheckBox(QString::fromUtf8("启用"), this);
    cost_corr_check_->setChecked(true);
    layout->addWidget(cost_corr_check_, row, 1, 1, 3);
}

void GeneratorWidget::SetupConnections() {
    connect(quick_mode_button_, &QPushButton::clicked, this, [this]() {
        is_quick_mode_ = true;
        quick_mode_button_->setChecked(true);
        manual_mode_button_->setChecked(false);
        OnModeChanged();
    });
    connect(manual_mode_button_, &QPushButton::clicked, this, [this]() {
        is_quick_mode_ = false;
        quick_mode_button_->setChecked(false);
        manual_mode_button_->setChecked(true);
        OnModeChanged();
    });

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
    quick_group_->setVisible(is_quick_mode_);
    manual_group_->setVisible(!is_quick_mode_);
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

    if (is_quick_mode_) {
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
