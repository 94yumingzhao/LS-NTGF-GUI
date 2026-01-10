// variables_panel.cpp - Variables browser panel implementation

#include "variables_panel.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QButtonGroup>
#include <QTableWidget>
#include <QPushButton>
#include <QLabel>
#include <QHeaderView>
#include <QFileDialog>
#include <QFile>
#include <QTextStream>
#include <QMessageBox>
#include <QColor>

VariablesPanel::VariablesPanel(QWidget* parent)
    : QWidget(parent) {
    SetupUi();
}

void VariablesPanel::SetupUi() {
    auto* layout = new QVBoxLayout(this);
    layout->setSpacing(8);
    layout->setContentsMargins(8, 8, 8, 8);

    // Top bar: variable selector buttons + export
    auto* top_layout = new QHBoxLayout();
    top_layout->setSpacing(4);

    var_button_group_ = new QButtonGroup(this);
    var_button_group_->setExclusive(true);

    // Variable button definitions
    struct VarDef { QString name; QString label; };
    QVector<VarDef> var_defs = {
        {"X", QString::fromUtf8("X-生产量")},
        {"Y", QString::fromUtf8("Y-启动")},
        {"L", QString::fromUtf8("L-跨期")},
        {"I", QString::fromUtf8("I-库存")},
        {"B", QString::fromUtf8("B-欠交")},
        {"U", QString::fromUtf8("U-未满足")}
    };

    QString button_style =
        "QPushButton { padding: 4px 8px; border: 1px solid #ced4da; border-radius: 3px; }"
        "QPushButton:checked { background-color: #228be6; color: white; border-color: #1971c2; }"
        "QPushButton:hover:!checked { background-color: #e9ecef; }";

    for (int i = 0; i < var_defs.size(); ++i) {
        auto* btn = new QPushButton(var_defs[i].label, this);
        btn->setCheckable(true);
        btn->setStyleSheet(button_style);
        var_button_group_->addButton(btn, i);
        var_buttons_.append(btn);
        var_names_.append(var_defs[i].name);
        top_layout->addWidget(btn);
    }
    var_buttons_[0]->setChecked(true);  // Default to X

    top_layout->addStretch();

    export_button_ = new QPushButton(QString::fromUtf8("导出CSV"), this);
    export_button_->setEnabled(false);
    export_button_->setFixedWidth(80);
    top_layout->addWidget(export_button_);

    layout->addLayout(top_layout);

    // Info label
    info_label_ = new QLabel("--", this);
    info_label_->setStyleSheet("color: #6c757d; font-size: 9pt;");
    layout->addWidget(info_label_);

    // Table
    table_ = new QTableWidget(this);
    table_->setAlternatingRowColors(true);
    table_->setStyleSheet(
        "QTableWidget { gridline-color: #dee2e6; }"
        "QTableWidget::item { padding: 2px; }"
    );
    table_->horizontalHeader()->setDefaultAlignment(Qt::AlignCenter);
    table_->verticalHeader()->setDefaultAlignment(Qt::AlignCenter);
    layout->addWidget(table_, 1);

    // Connections
    connect(var_button_group_, &QButtonGroup::idClicked,
            this, &VariablesPanel::OnVariableButtonClicked);
    connect(export_button_, &QPushButton::clicked,
            this, &VariablesPanel::OnExportCsv);
}

void VariablesPanel::LoadData(const QJsonObject& json) {
    variables_ = json["variables"].toObject();
    export_button_->setEnabled(!variables_.isEmpty());

    // Display currently selected variable
    if (!variables_.isEmpty()) {
        int checked_id = var_button_group_->checkedId();
        if (checked_id >= 0 && checked_id < var_names_.size()) {
            DisplayVariable(var_names_[checked_id]);
        }
    }
}

void VariablesPanel::Clear() {
    variables_ = QJsonObject();
    table_->clear();
    table_->setRowCount(0);
    table_->setColumnCount(0);
    info_label_->setText("--");
    export_button_->setEnabled(false);
}

void VariablesPanel::OnVariableButtonClicked(int id) {
    if (!variables_.isEmpty() && id >= 0 && id < var_names_.size()) {
        DisplayVariable(var_names_[id]);
    }
}

void VariablesPanel::DisplayVariable(const QString& name) {
    current_var_ = name;

    if (!variables_.contains(name)) {
        table_->clear();
        table_->setRowCount(0);
        table_->setColumnCount(0);
        info_label_->setText(QString::fromUtf8("变量 %1 不存在").arg(name));
        return;
    }

    QJsonObject var_obj = variables_[name].toObject();
    QString desc = var_obj["description"].toString();
    QJsonArray dims = var_obj["dimensions"].toArray();
    QJsonArray data = var_obj["data"].toArray();

    if (dims.size() == 2) {
        // 2D variable
        int rows = dims[0].toInt();
        int cols = dims[1].toInt();
        info_label_->setText(QString("%1 [%2 x %3]").arg(desc).arg(rows).arg(cols));

        QString row_prefix = (name == "Y" || name == "L") ? "G" :
                             (name == "I") ? "F" : "i";
        PopulateTable2D(data, rows, cols, row_prefix, "t");
    } else if (dims.size() == 1) {
        // 1D variable
        int rows = dims[0].toInt();
        info_label_->setText(QString("%1 [%2]").arg(desc).arg(rows));
        PopulateTable1D(data, "i");
    }
}

void VariablesPanel::PopulateTable2D(const QJsonArray& data, int rows, int cols,
                                      const QString& row_prefix,
                                      const QString& col_prefix) {
    table_->clear();
    table_->setRowCount(rows + 1);  // +1 for totals
    table_->setColumnCount(cols + 1);  // +1 for totals

    // Set headers
    QStringList h_headers;
    for (int c = 0; c < cols; ++c) {
        h_headers << QString("%1=%2").arg(col_prefix).arg(c + 1);
    }
    h_headers << QString::fromUtf8("合计");
    table_->setHorizontalHeaderLabels(h_headers);

    QStringList v_headers;
    for (int r = 0; r < rows; ++r) {
        v_headers << QString("%1%2").arg(row_prefix).arg(r + 1);
    }
    v_headers << QString::fromUtf8("合计");
    table_->setVerticalHeaderLabels(v_headers);

    // Fill data and calculate totals
    QVector<double> col_totals(cols, 0.0);
    double grand_total = 0.0;

    for (int r = 0; r < rows && r < data.size(); ++r) {
        QJsonArray row_data = data[r].toArray();
        double row_total = 0.0;

        for (int c = 0; c < cols && c < row_data.size(); ++c) {
            double val = row_data[c].toDouble();
            auto* item = new QTableWidgetItem(QString::number(val, 'f', 0));
            item->setTextAlignment(Qt::AlignCenter);

            // Highlight non-zero values
            if (val > 0.5) {
                item->setBackground(GetHighlightColor());
            }

            table_->setItem(r, c, item);
            row_total += val;
            col_totals[c] += val;
        }

        // Row total
        auto* row_total_item = new QTableWidgetItem(QString::number(row_total, 'f', 0));
        row_total_item->setTextAlignment(Qt::AlignCenter);
        row_total_item->setBackground(QColor("#f1f3f5"));
        row_total_item->setFont(QFont("", -1, QFont::Bold));
        table_->setItem(r, cols, row_total_item);
        grand_total += row_total;
    }

    // Column totals row
    for (int c = 0; c < cols; ++c) {
        auto* item = new QTableWidgetItem(QString::number(col_totals[c], 'f', 0));
        item->setTextAlignment(Qt::AlignCenter);
        item->setBackground(QColor("#f1f3f5"));
        item->setFont(QFont("", -1, QFont::Bold));
        table_->setItem(rows, c, item);
    }

    // Grand total
    auto* grand_item = new QTableWidgetItem(QString::number(grand_total, 'f', 0));
    grand_item->setTextAlignment(Qt::AlignCenter);
    grand_item->setBackground(QColor("#e9ecef"));
    grand_item->setFont(QFont("", -1, QFont::Bold));
    table_->setItem(rows, cols, grand_item);

    table_->resizeColumnsToContents();
}

void VariablesPanel::PopulateTable1D(const QJsonArray& data,
                                      const QString& row_prefix) {
    table_->clear();
    int rows = data.size();
    table_->setRowCount(rows + 1);
    table_->setColumnCount(1);

    table_->setHorizontalHeaderLabels({QString::fromUtf8("值")});

    QStringList v_headers;
    for (int r = 0; r < rows; ++r) {
        v_headers << QString("%1%2").arg(row_prefix).arg(r + 1);
    }
    v_headers << QString::fromUtf8("合计");
    table_->setVerticalHeaderLabels(v_headers);

    double total = 0.0;
    for (int r = 0; r < rows; ++r) {
        double val = data[r].toDouble();
        auto* item = new QTableWidgetItem(QString::number(val, 'f', 0));
        item->setTextAlignment(Qt::AlignCenter);
        if (val > 0.5) {
            item->setBackground(GetHighlightColor());
        }
        table_->setItem(r, 0, item);
        total += val;
    }

    auto* total_item = new QTableWidgetItem(QString::number(total, 'f', 0));
    total_item->setTextAlignment(Qt::AlignCenter);
    total_item->setBackground(QColor("#f1f3f5"));
    total_item->setFont(QFont("", -1, QFont::Bold));
    table_->setItem(rows, 0, total_item);

    table_->resizeColumnsToContents();
}

void VariablesPanel::OnExportCsv() {
    if (current_var_.isEmpty() || !variables_.contains(current_var_)) {
        return;
    }

    QString filename = QFileDialog::getSaveFileName(
        this,
        QString::fromUtf8("导出CSV"),
        QString("%1.csv").arg(current_var_),
        "CSV files (*.csv)");

    if (filename.isEmpty()) {
        return;
    }

    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, QString::fromUtf8("错误"),
                             QString::fromUtf8("无法创建文件: %1").arg(filename));
        return;
    }

    QTextStream out(&file);

    // Write headers
    QStringList headers;
    for (int c = 0; c < table_->columnCount(); ++c) {
        auto* item = table_->horizontalHeaderItem(c);
        headers << (item ? item->text() : QString::number(c));
    }
    out << "," << headers.join(",") << "\n";

    // Write data
    for (int r = 0; r < table_->rowCount(); ++r) {
        auto* v_item = table_->verticalHeaderItem(r);
        QString row_header = v_item ? v_item->text() : QString::number(r);

        QStringList row_data;
        row_data << row_header;
        for (int c = 0; c < table_->columnCount(); ++c) {
            auto* item = table_->item(r, c);
            row_data << (item ? item->text() : "");
        }
        out << row_data.join(",") << "\n";
    }

    file.close();
    QMessageBox::information(this, QString::fromUtf8("成功"),
                             QString::fromUtf8("已导出到: %1").arg(filename));
}

QColor VariablesPanel::GetHighlightColor() const {
    // 根据变量类型返回对应的高亮颜色
    if (current_var_ == "Y") {
        return QColor("#dbe4ff");  // 蓝色 - 启动
    } else if (current_var_ == "L") {
        return QColor("#d3f9d8");  // 绿色 - 跨期
    } else if (current_var_ == "X") {
        return QColor("#fff3bf");  // 黄色 - 生产量
    } else if (current_var_ == "I") {
        return QColor("#e5dbff");  // 紫色 - 库存
    } else if (current_var_ == "B" || current_var_ == "U") {
        return QColor("#ffe3e3");  // 红色 - 欠交/未满足
    }
    return QColor("#e7f5ff");  // 默认浅蓝
}
