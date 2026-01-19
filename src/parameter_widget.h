// parameter_widget.h - Parameter Configuration Widget

#ifndef PARAMETER_WIDGET_H_
#define PARAMETER_WIDGET_H_

#include <QGroupBox>

class QDoubleSpinBox;
class QSpinBox;
class QPushButton;
class QComboBox;
class QCheckBox;
class QWidget;
class QVBoxLayout;

// 算法类型 (与 solver_worker.h 保持一致)
enum class AlgorithmType;

class ParameterWidget : public QGroupBox {
    Q_OBJECT

public:
    explicit ParameterWidget(QWidget* parent = nullptr);

    // Basic parameters
    int GetAlgorithmIndex() const;
    double GetRuntimeLimit() const;
    int GetUPenalty() const;
    int GetBPenalty() const;
    bool GetMergeEnabled() const;
    double GetBigOrderThreshold() const;
    int GetMachineCapacity() const;

    // RF parameters
    int GetRFWindow() const;
    int GetRFStep() const;
    double GetRFTime() const;
    int GetRFRetries() const;

    // FO parameters (for RFO)
    int GetFOWindow() const;
    int GetFOStep() const;
    int GetFORounds() const;
    int GetFOBuffer() const;
    double GetFOTime() const;

    // RR parameters
    double GetRRCapacity() const;
    double GetRRBonus() const;

    // LR parameters
    int GetLRMaxIter() const;
    double GetLRAlpha0() const;
    double GetLRDecay() const;
    double GetLRTol() const;

signals:
    void AlgorithmChanged(int index);

public slots:
    void ResetDefaults();

private slots:
    void OnAlgorithmChanged(int index);

private:
    void SetupUi();
    void SetupBasicParams(QVBoxLayout* layout);
    void SetupAlgorithmParams(QVBoxLayout* layout);
    void UpdateParamGroupStates(int algorithmIndex);

    // Basic parameters
    QComboBox* algorithm_combo_;
    QDoubleSpinBox* runtime_limit_spin_;
    QSpinBox* machine_capacity_spin_;
    QSpinBox* u_penalty_spin_;
    QSpinBox* b_penalty_spin_;
    QCheckBox* merge_checkbox_;
    QDoubleSpinBox* big_order_threshold_spin_;
    QPushButton* reset_button_;

    // RF parameters
    QGroupBox* rf_group_;
    QSpinBox* rf_window_spin_;
    QSpinBox* rf_step_spin_;
    QDoubleSpinBox* rf_time_spin_;
    QSpinBox* rf_retries_spin_;

    // FO parameters
    QGroupBox* fo_group_;
    QSpinBox* fo_window_spin_;
    QSpinBox* fo_step_spin_;
    QSpinBox* fo_rounds_spin_;
    QSpinBox* fo_buffer_spin_;
    QDoubleSpinBox* fo_time_spin_;

    // RR parameters
    QGroupBox* rr_group_;
    QDoubleSpinBox* rr_capacity_spin_;
    QDoubleSpinBox* rr_bonus_spin_;

    // LR parameters
    QGroupBox* lr_group_;
    QSpinBox* lr_maxiter_spin_;
    QDoubleSpinBox* lr_alpha0_spin_;
    QDoubleSpinBox* lr_decay_spin_;
    QDoubleSpinBox* lr_tol_spin_;
};

#endif  // PARAMETER_WIDGET_H_
