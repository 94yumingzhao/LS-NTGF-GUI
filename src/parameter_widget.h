// parameter_widget.h - Parameter Configuration Widget

#ifndef PARAMETER_WIDGET_H_
#define PARAMETER_WIDGET_H_

#include <QGroupBox>

class QDoubleSpinBox;
class QSpinBox;
class QPushButton;

class ParameterWidget : public QGroupBox {
    Q_OBJECT

public:
    explicit ParameterWidget(QWidget* parent = nullptr);

    double GetRuntimeLimit() const;
    int GetUPenalty() const;
    int GetBPenalty() const;
    double GetBigOrderThreshold() const;

public slots:
    void ResetDefaults();

private:
    void SetupUi();

    QDoubleSpinBox* runtime_limit_spin_;
    QSpinBox* u_penalty_spin_;
    QSpinBox* b_penalty_spin_;
    QDoubleSpinBox* big_order_threshold_spin_;
    QPushButton* reset_button_;
};

#endif  // PARAMETER_WIDGET_H_
