// generator_widget.h - Instance Generator Widget
//
// Provides UI for configuring and generating test instances

#ifndef GENERATOR_WIDGET_H_
#define GENERATOR_WIDGET_H_

#include <QWidget>
#include "difficulty_mapper.h"

class QComboBox;
class QSpinBox;
class QDoubleSpinBox;
class QCheckBox;
class QPushButton;
class QLabel;
class QLineEdit;
class QGroupBox;
class QButtonGroup;
class QRadioButton;

class GeneratorWidget : public QWidget {
    Q_OBJECT

public:
    explicit GeneratorWidget(QWidget* parent = nullptr);

    // Get current configuration
    GeneratorConfig GetConfig() const;

    // Set output path
    void SetOutputPath(const QString& path);

signals:
    void GenerateRequested(const GeneratorConfig& config);
    void ConfigChanged();

private slots:
    void OnModeChanged();
    void OnDifficultyChanged();
    void OnScaleChanged();
    void OnBrowseOutput();
    void OnGenerateClicked();
    void UpdatePreview();

private:
    void SetupUi();
    void SetupQuickModeUi(QGroupBox* group);
    void SetupManualModeUi(QGroupBox* group);
    void SetupConnections();
    void ApplyPreset(const GeneratorConfig& config);

    // Mode selection
    QComboBox* mode_combo_;

    // Quick mode widgets
    QGroupBox* quick_group_;
    QButtonGroup* difficulty_button_group_;
    QRadioButton* easy_radio_;
    QRadioButton* medium_radio_;
    QRadioButton* hard_radio_;
    QRadioButton* expert_radio_;

    QButtonGroup* scale_button_group_;
    QRadioButton* small_radio_;
    QRadioButton* medium_scale_radio_;
    QRadioButton* large_radio_;

    // Manual mode widgets
    QGroupBox* manual_group_;
    QSpinBox* n_spin_;
    QSpinBox* t_spin_;
    QSpinBox* f_spin_;
    QSpinBox* g_spin_;
    QDoubleSpinBox* capacity_spin_;
    QSpinBox* offset_spin_;
    QDoubleSpinBox* demand_cv_spin_;
    QDoubleSpinBox* peak_ratio_spin_;
    QDoubleSpinBox* peak_mult_spin_;
    QDoubleSpinBox* urgent_spin_;
    QDoubleSpinBox* flexible_spin_;
    QCheckBox* cost_corr_check_;
    QSpinBox* zoom_spin_;

    // Common widgets
    QSpinBox* seed_spin_;
    QSpinBox* count_spin_;
    QLineEdit* output_edit_;
    QPushButton* browse_button_;

    // Preview
    QLabel* preview_label_;

    // Generate button
    QPushButton* generate_button_;
};

#endif  // GENERATOR_WIDGET_H_
