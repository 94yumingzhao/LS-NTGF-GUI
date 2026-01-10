// analysis_widget.cpp - Main analysis widget implementation

#include "analysis_widget.h"
#include "panels/overview_panel.h"
#include "panels/capacity_panel.h"
#include "panels/setup_panel.h"
#include "panels/variables_panel.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QPushButton>
#include <QLabel>
#include <QTabWidget>
#include <QFileDialog>
#include <QMessageBox>
#include <QFile>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QFileInfo>

AnalysisWidget::AnalysisWidget(QWidget* parent)
    : QWidget(parent) {
    SetupUi();
}

void AnalysisWidget::SetupUi() {
    auto* layout = new QVBoxLayout(this);
    layout->setSpacing(8);
    layout->setContentsMargins(4, 4, 4, 4);

    // File operations group
    auto* file_group = new QGroupBox(this);
    auto* file_layout = new QVBoxLayout(file_group);
    file_layout->setSpacing(4);

    auto* button_layout = new QHBoxLayout();
    open_button_ = new QPushButton(QString::fromUtf8("打开JSON..."), this);
    clear_button_ = new QPushButton(QString::fromUtf8("清除"), this);
    clear_button_->setEnabled(false);

    button_layout->addWidget(open_button_);
    button_layout->addWidget(clear_button_);
    file_layout->addLayout(button_layout);

    file_label_ = new QLabel(QString::fromUtf8("未加载文件"), this);
    file_label_->setStyleSheet("color: #6c757d; font-size: 9pt;");
    file_label_->setWordWrap(true);
    file_layout->addWidget(file_label_);

    layout->addWidget(file_group);

    // Analysis tabs
    tabs_ = new QTabWidget(this);

    overview_panel_ = new OverviewPanel(this);
    capacity_panel_ = new CapacityPanel(this);
    setup_panel_ = new SetupPanel(this);
    variables_panel_ = new VariablesPanel(this);

    tabs_->addTab(overview_panel_, QString::fromUtf8("求解结果概览"));
    tabs_->addTab(capacity_panel_, QString::fromUtf8("产能利用概览"));
    tabs_->addTab(setup_panel_, QString::fromUtf8("启动/跨期情况"));
    tabs_->addTab(variables_panel_, QString::fromUtf8("决策变量结果"));

    layout->addWidget(tabs_, 1);

    // Connections
    connect(open_button_, &QPushButton::clicked, this, &AnalysisWidget::OnOpenFile);
    connect(clear_button_, &QPushButton::clicked, this, &AnalysisWidget::OnClearData);
}

void AnalysisWidget::OnOpenFile() {
    QString default_dir = "D:/YM-Code/LS-NTGF-All/results";
    QString path = QFileDialog::getOpenFileName(
        this,
        QString::fromUtf8("打开结果文件"),
        default_dir,
        "JSON files (*.json);;All files (*)");

    if (path.isEmpty()) {
        return;
    }

    LoadJsonFile(path);
}

bool AnalysisWidget::LoadJsonFile(const QString& path) {
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, QString::fromUtf8("错误"),
                             QString::fromUtf8("无法打开文件: %1").arg(path));
        return false;
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(data, &error);

    if (error.error != QJsonParseError::NoError) {
        QMessageBox::warning(this, QString::fromUtf8("JSON 解析错误"),
                             QString::fromUtf8("位置 %1: %2")
                                 .arg(error.offset)
                                 .arg(error.errorString()));
        return false;
    }

    if (!doc.isObject()) {
        QMessageBox::warning(this, QString::fromUtf8("错误"),
                             QString::fromUtf8("JSON 文件格式不正确"));
        return false;
    }

    json_data_ = doc.object();
    current_file_ = path;

    // Update UI
    QFileInfo info(path);
    file_label_->setText(info.fileName());
    file_label_->setToolTip(path);
    clear_button_->setEnabled(true);

    UpdateAllPanels();
    return true;
}

void AnalysisWidget::Clear() {
    json_data_ = QJsonObject();
    current_file_.clear();

    file_label_->setText(QString::fromUtf8("未加载文件"));
    file_label_->setToolTip("");
    clear_button_->setEnabled(false);

    overview_panel_->Clear();
    capacity_panel_->Clear();
    setup_panel_->Clear();
    variables_panel_->Clear();
}

void AnalysisWidget::OnClearData() {
    Clear();
}

void AnalysisWidget::UpdateAllPanels() {
    if (json_data_.isEmpty()) {
        return;
    }

    overview_panel_->LoadData(json_data_);
    capacity_panel_->LoadData(json_data_);
    setup_panel_->LoadData(json_data_);
    variables_panel_->LoadData(json_data_);
}
