// setup_panel.cpp - Setup/Carryover visualization panel implementation

#include "setup_panel.h"
#include "../widgets/heatmap.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QButtonGroup>
#include <QRadioButton>
#include <QLabel>
#include <QScrollArea>
#include <QJsonArray>

SetupPanel::SetupPanel(QWidget* parent)
    : QWidget(parent) {
    SetupUi();
}

void SetupPanel::SetupUi() {
    auto* layout = new QVBoxLayout(this);
    layout->setSpacing(8);
    layout->setContentsMargins(8, 8, 8, 8);

    // Matrix selector
    auto* selector_layout = new QHBoxLayout();

    auto* y_radio = new QRadioButton(QString::fromUtf8("Y - 启动决策"), this);
    auto* l_radio = new QRadioButton(QString::fromUtf8("L - 跨期决策"), this);
    y_radio->setChecked(true);

    matrix_group_ = new QButtonGroup(this);
    matrix_group_->addButton(y_radio, 0);
    matrix_group_->addButton(l_radio, 1);

    selector_layout->addWidget(y_radio);
    selector_layout->addWidget(l_radio);
    selector_layout->addStretch();

    layout->addLayout(selector_layout);

    // Info label
    info_label_ = new QLabel("--", this);
    info_label_->setStyleSheet("color: #6c757d; font-size: 9pt;");
    layout->addWidget(info_label_);

    // Scrollable heatmap area
    scroll_area_ = new QScrollArea(this);
    scroll_area_->setWidgetResizable(true);
    scroll_area_->setStyleSheet("QScrollArea { border: 1px solid #dee2e6; }");

    heatmap_ = new Heatmap(this);
    scroll_area_->setWidget(heatmap_);

    layout->addWidget(scroll_area_, 1);

    // Connections
    connect(matrix_group_, QOverload<int>::of(&QButtonGroup::idClicked),
            this, &SetupPanel::OnMatrixChanged);
}

void SetupPanel::LoadData(const QJsonObject& json) {
    variables_ = json["variables"].toObject();

    // Display currently selected matrix
    int current_id = matrix_group_->checkedId();
    DisplayMatrix(current_id == 0 ? "Y" : "L");
}

void SetupPanel::Clear() {
    variables_ = QJsonObject();
    heatmap_->Clear();
    info_label_->setText("--");
}

void SetupPanel::OnMatrixChanged(int id) {
    DisplayMatrix(id == 0 ? "Y" : "L");
}

void SetupPanel::DisplayMatrix(const QString& name) {
    if (!variables_.contains(name)) {
        heatmap_->Clear();
        info_label_->setText(QString::fromUtf8("变量 %1 不存在").arg(name));
        return;
    }

    QJsonObject var_obj = variables_[name].toObject();
    QString desc = var_obj["description"].toString();
    QJsonArray dims = var_obj["dimensions"].toArray();
    QJsonArray data = var_obj["data"].toArray();

    if (dims.size() != 2) {
        heatmap_->Clear();
        info_label_->setText(QString::fromUtf8("数据维度错误"));
        return;
    }

    int rows = dims[0].toInt();
    int cols = dims[1].toInt();

    // Count non-zeros
    int nonzero_count = 0;
    QVector<QVector<int>> matrix_data;
    for (int r = 0; r < data.size(); ++r) {
        QJsonArray row_arr = data[r].toArray();
        QVector<int> row_vec;
        for (int c = 0; c < row_arr.size(); ++c) {
            int val = row_arr[c].toInt();
            row_vec.append(val);
            if (val > 0) {
                nonzero_count++;
            }
        }
        matrix_data.append(row_vec);
    }

    info_label_->setText(QString("%1 [%2 x %3] - %4 = %5")
        .arg(desc)
        .arg(rows)
        .arg(cols)
        .arg(QString::fromUtf8("非零元素"))
        .arg(nonzero_count));

    // Set colors based on matrix type
    if (name == "Y") {
        heatmap_->SetColors(QColor("#f1f3f5"), QColor("#228be6"));  // Blue for setup
    } else {
        heatmap_->SetColors(QColor("#f1f3f5"), QColor("#40c057"));  // Green for carryover
    }

    heatmap_->SetData(matrix_data);
}
