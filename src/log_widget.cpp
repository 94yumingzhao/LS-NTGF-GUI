// log_widget.cpp - Log Output Widget Implementation

#include "log_widget.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTextEdit>
#include <QPushButton>
#include <QLabel>
#include <QTimer>
#include <QDateTime>
#include <QScrollBar>

LogWidget::LogWidget(QWidget* parent)
    : QGroupBox(QString::fromUtf8("运行日志"), parent)
    , timer_running_(false)
    , stopped_elapsed_(0) {
    auto* layout = new QVBoxLayout(this);

    // 顶部栏：计时器 + 清除按钮
    auto* top_layout = new QHBoxLayout();

    timer_label_ = new QLabel(QString::fromUtf8("耗时: 00:00.0"), this);
    timer_label_->setStyleSheet("color: #495057; font-family: Consolas; font-size: 10pt;");
    top_layout->addWidget(timer_label_);

    top_layout->addStretch();

    clear_button_ = new QPushButton(QString::fromUtf8("清除"), this);
    clear_button_->setFixedWidth(60);
    top_layout->addWidget(clear_button_);

    layout->addLayout(top_layout);

    text_edit_ = new QTextEdit(this);
    text_edit_->setReadOnly(true);
    text_edit_->setFont(QFont("Consolas", 9));
    text_edit_->setLineWrapMode(QTextEdit::NoWrap);

    layout->addWidget(text_edit_);

    // 定时更新显示（100ms）
    update_timer_ = new QTimer(this);
    connect(update_timer_, &QTimer::timeout, this, &LogWidget::UpdateTimerDisplay);

    connect(clear_button_, &QPushButton::clicked, this, &LogWidget::ClearLog);
}

void LogWidget::ClearLog() {
    text_edit_->clear();
    ResetTimer();
}

QString LogWidget::GetLogText() const {
    return text_edit_->toPlainText();
}

void LogWidget::StartTimer() {
    elapsed_timer_.start();
    timer_running_ = true;
    stopped_elapsed_ = 0;
    update_timer_->start(100);
    UpdateTimerDisplay();
}

void LogWidget::StopTimer() {
    if (timer_running_) {
        stopped_elapsed_ = elapsed_timer_.elapsed();
        timer_running_ = false;
        update_timer_->stop();
        UpdateTimerDisplay();
    }
}

void LogWidget::ResetTimer() {
    timer_running_ = false;
    stopped_elapsed_ = 0;
    update_timer_->stop();
    timer_label_->setText(QString::fromUtf8("耗时: 00:00.0"));
}

void LogWidget::UpdateTimerDisplay() {
    qint64 elapsed = timer_running_ ? elapsed_timer_.elapsed() : stopped_elapsed_;
    timer_label_->setText(QString::fromUtf8("耗时: %1").arg(FormatElapsedTime(elapsed)));
}

QString LogWidget::FormatElapsedTime(qint64 ms) const {
    int total_secs = ms / 1000;
    int mins = total_secs / 60;
    int secs = total_secs % 60;
    int tenths = (ms % 1000) / 100;

    if (mins > 0) {
        return QString("%1:%2.%3")
            .arg(mins, 2, 10, QChar('0'))
            .arg(secs, 2, 10, QChar('0'))
            .arg(tenths);
    } else {
        return QString("00:%1.%2")
            .arg(secs, 2, 10, QChar('0'))
            .arg(tenths);
    }
}

void LogWidget::AppendLog(const QString& message) {
    QString formatted;

    // 如果消息已经包含时间戳 [YYYY-MM-DD HH:MM:SS]，则不再添加
    if (message.startsWith("[20") && message.length() > 21 && message[20] == ']') {
        formatted = message;
    } else {
        // 添加时间戳（包括空行）
        QString timestamp = GetTimestamp();
        formatted = QString("[%1] %2").arg(timestamp, message);
    }
    text_edit_->append(formatted);

    // Auto-scroll to bottom
    QScrollBar* scrollbar = text_edit_->verticalScrollBar();
    scrollbar->setValue(scrollbar->maximum());
}

QString LogWidget::GetTimestamp() const {
    return QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
}
