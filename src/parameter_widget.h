// parameter_widget.h - Parameter Configuration Widget

#ifndef PARAMETER_WIDGET_H_
#define PARAMETER_WIDGET_H_

#include <QGroupBox>

class QDoubleSpinBox;
class QSpinBox;
class QPushButton;
class QComboBox;
class QCheckBox;

// 算法类型 (与 solver_worker.h 保持一致)
enum class AlgorithmType;

class ParameterWidget : public QGroupBox {
    Q_OBJECT

public:
    explicit ParameterWidget(QWidget* parent = nullptr);

    int GetAlgorithmIndex() const;
    double GetRuntimeLimit() const;
    int GetUPenalty() const;
    int GetBPenalty() const;
    bool GetMergeEnabled() const;
    double GetBigOrderThreshold() const;

signals:
    void AlgorithmChanged(int index);

public slots:
    void ResetDefaults();

private:
    void SetupUi();

    QComboBox* algorithm_combo_;
    QDoubleSpinBox* runtime_limit_spin_;
    QSpinBox* u_penalty_spin_;
    QSpinBox* b_penalty_spin_;
    QCheckBox* merge_checkbox_;
    QDoubleSpinBox* big_order_threshold_spin_;
    QPushButton* reset_button_;
};

#endif  // PARAMETER_WIDGET_H_
