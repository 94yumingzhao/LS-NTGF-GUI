// log_widget.cpp - Log Output Widget Implementation

#include "log_widget.h"

#include <QVBoxLayout>
#include <QTextEdit>
#include <QDateTime>
#include <QScrollBar>

LogWidget::LogWidget(QWidget* parent)
    : QGroupBox(QString::fromUtf8("运行日志"), parent) {
    auto* layout = new QVBoxLayout(this);

    text_edit_ = new QTextEdit(this);
    text_edit_->setReadOnly(true);
    text_edit_->setFont(QFont("Consolas", 9));
    text_edit_->setLineWrapMode(QTextEdit::NoWrap);

    layout->addWidget(text_edit_);
}

void LogWidget::ClearLog() {
    text_edit_->clear();
}

QString LogWidget::GetLogText() const {
    return text_edit_->toPlainText();
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
