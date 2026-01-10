// metric_card.cpp - Metric display card widget implementation

#include "metric_card.h"
#include <QVBoxLayout>
#include <QLabel>

MetricCard::MetricCard(const QString& title, QWidget* parent)
    : QFrame(parent)
    , title_(title) {
    SetupUi();
}

void MetricCard::SetupUi() {
    setFrameStyle(QFrame::StyledPanel | QFrame::Raised);
    setStyleSheet(
        "MetricCard {"
        "  background-color: #f8f9fa;"
        "  border: 1px solid #dee2e6;"
        "  border-radius: 4px;"
        "}"
    );
    setMinimumSize(80, 60);

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(8, 6, 8, 6);
    layout->setSpacing(2);

    title_label_ = new QLabel(title_, this);
    title_label_->setStyleSheet("color: #6c757d; font-size: 9pt;");
    title_label_->setAlignment(Qt::AlignCenter);

    value_label_ = new QLabel("--", this);
    value_label_->setStyleSheet("color: #212529; font-size: 12pt; font-weight: bold;");
    value_label_->setAlignment(Qt::AlignCenter);

    layout->addWidget(title_label_);
    layout->addWidget(value_label_);
}

void MetricCard::SetValue(const QString& value) {
    value_label_->setText(value);
}

void MetricCard::SetValue(double value, int precision) {
    value_label_->setText(QString::number(value, 'f', precision));
}

void MetricCard::SetValue(int value) {
    value_label_->setText(QString::number(value));
}

void MetricCard::SetTitle(const QString& title) {
    title_ = title;
    title_label_->setText(title);
}

void MetricCard::SetHighlight(bool highlight) {
    if (highlight) {
        setStyleSheet(
            "MetricCard {"
            "  background-color: #e7f5ff;"
            "  border: 1px solid #74c0fc;"
            "  border-radius: 4px;"
            "}"
        );
    } else {
        setStyleSheet(
            "MetricCard {"
            "  background-color: #f8f9fa;"
            "  border: 1px solid #dee2e6;"
            "  border-radius: 4px;"
            "}"
        );
    }
}
