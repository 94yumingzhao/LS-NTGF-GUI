// log_widget.h - Log Output Widget

#ifndef LOG_WIDGET_H_
#define LOG_WIDGET_H_

#include <QGroupBox>
#include <QString>
#include <QElapsedTimer>

class QTextEdit;
class QPushButton;
class QLabel;
class QTimer;

class LogWidget : public QGroupBox {
    Q_OBJECT

public:
    explicit LogWidget(QWidget* parent = nullptr);

    void ClearLog();
    QString GetLogText() const;

    // 计时器控制
    void StartTimer();
    void StopTimer();
    void ResetTimer();

public slots:
    void AppendLog(const QString& message);

private slots:
    void UpdateTimerDisplay();

private:
    QString GetTimestamp() const;
    QString FormatElapsedTime(qint64 ms) const;

    QTextEdit* text_edit_;
    QPushButton* clear_button_;
    QLabel* timer_label_;
    QTimer* update_timer_;
    QElapsedTimer elapsed_timer_;
    bool timer_running_;
    qint64 stopped_elapsed_;  // 停止时的累计时间
};

#endif  // LOG_WIDGET_H_
