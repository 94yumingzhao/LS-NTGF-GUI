// cplex_settings_widget.cpp - CPLEX Settings Widget Implementation

#include "cplex_settings_widget.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QSpinBox>
#include <QPushButton>
#include <QFileDialog>

CplexSettingsWidget::CplexSettingsWidget(QWidget* parent)
    : QWidget(parent) {
    SetupUi();
}

void CplexSettingsWidget::SetupUi() {
    auto* layout = new QHBoxLayout(this);
    layout->setContentsMargins(4, 2, 4, 2);
    layout->setSpacing(8);

    // Cache Directory
    auto* workdir_label = new QLabel(QString::fromUtf8("CPLEX缓存目录:"), this);
    workdir_edit_ = new QLineEdit(this);
    workdir_edit_->setText("D:/CPLEX_Temp");
    workdir_edit_->setMinimumWidth(150);
    workdir_browse_ = new QPushButton("...", this);
    workdir_browse_->setFixedWidth(28);
    connect(workdir_browse_, &QPushButton::clicked,
            this, &CplexSettingsWidget::OnBrowseWorkDir);

    layout->addWidget(workdir_label);
    layout->addWidget(workdir_edit_);
    layout->addWidget(workdir_browse_);

    layout->addSpacing(16);

    // Cache Memory Limit
    auto* workmem_label = new QLabel(QString::fromUtf8("CPLEX缓存限制:"), this);
    workmem_spin_ = new QSpinBox(this);
    workmem_spin_->setRange(512, 65536);
    workmem_spin_->setValue(4096);
    workmem_spin_->setSuffix(" MB");
    workmem_spin_->setSingleStep(1024);

    layout->addWidget(workmem_label);
    layout->addWidget(workmem_spin_);

    layout->addSpacing(16);

    // CPLEX Threads
    auto* threads_label = new QLabel(QString::fromUtf8("CPLEX线程数:"), this);
    threads_spin_ = new QSpinBox(this);
    threads_spin_->setRange(0, 128);
    threads_spin_->setValue(0);
    threads_spin_->setSpecialValueText(QString::fromUtf8("自动"));
    threads_spin_->setToolTip(QString::fromUtf8("0 = 自动选择CPLEX线程数"));

    layout->addWidget(threads_label);
    layout->addWidget(threads_spin_);

    layout->addStretch();
}

void CplexSettingsWidget::OnBrowseWorkDir() {
    QString dir = QFileDialog::getExistingDirectory(
        this,
        QString::fromUtf8("选择CPLEX缓存目录"),
        workdir_edit_->text());
    if (!dir.isEmpty()) {
        workdir_edit_->setText(dir);
    }
}

QString CplexSettingsWidget::GetWorkDir() const {
    return workdir_edit_->text();
}

int CplexSettingsWidget::GetWorkMem() const {
    return workmem_spin_->value();
}

int CplexSettingsWidget::GetThreads() const {
    return threads_spin_->value();
}
