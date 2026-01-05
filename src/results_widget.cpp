// results_widget.cpp - Results Display Widget Implementation

#include "results_widget.h"

#include <QVBoxLayout>
#include <QTableWidget>
#include <QHeaderView>
#include <QFile>
#include <QTextStream>

ResultsWidget::ResultsWidget(QWidget* parent)
    : QGroupBox(QString::fromUtf8("求解结果"), parent)
    , has_results_(false) {
    SetupUi();
}

void ResultsWidget::SetupUi() {
    auto* layout = new QVBoxLayout(this);

    table_ = new QTableWidget(3, 4, this);
    table_->setHorizontalHeaderLabels({
        QString::fromUtf8("阶段"),
        QString::fromUtf8("目标值"),
        QString::fromUtf8("耗时"),
        QString::fromUtf8("Gap")
    });
    table_->verticalHeader()->setVisible(false);
    table_->horizontalHeader()->setStretchLastSection(true);
    table_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table_->setSelectionBehavior(QAbstractItemView::SelectRows);

    // Set column widths
    table_->setColumnWidth(0, 100);
    table_->setColumnWidth(1, 150);
    table_->setColumnWidth(2, 100);
    table_->setColumnWidth(3, 80);

    // Initialize rows
    QStringList stage_names = {
        QString::fromUtf8("阶段 1"),
        QString::fromUtf8("阶段 2"),
        QString::fromUtf8("阶段 3")
    };
    for (int i = 0; i < 3; ++i) {
        table_->setItem(i, 0, new QTableWidgetItem(stage_names[i]));
        table_->setItem(i, 1, new QTableWidgetItem("--"));
        table_->setItem(i, 2, new QTableWidgetItem("--"));
        table_->setItem(i, 3, new QTableWidgetItem("--"));
    }

    layout->addWidget(table_);
}

void ResultsWidget::ClearResults() {
    for (int i = 0; i < 3; ++i) {
        table_->item(i, 1)->setText("--");
        table_->item(i, 2)->setText("--");
        table_->item(i, 3)->setText("--");
    }
    has_results_ = false;
}

void ResultsWidget::SetStageResult(int row, double objective, double runtime, double gap) {
    if (row >= 0 && row < 3) {
        table_->item(row, 1)->setText(QString::number(objective, 'f', 2));
        table_->item(row, 2)->setText(QString("%1s").arg(runtime, 0, 'f', 3));
        table_->item(row, 3)->setText(QString("%1%").arg(gap * 100, 0, 'f', 4));
        has_results_ = true;
    }
}

bool ResultsWidget::HasResults() const {
    return has_results_;
}

bool ResultsWidget::ExportToCsv(const QString& path) const {
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }

    QTextStream out(&file);
    out.setEncoding(QStringConverter::Utf8);
    out << QString::fromUtf8("阶段,目标值,耗时(s),Gap\n");

    for (int i = 0; i < 3; ++i) {
        out << table_->item(i, 0)->text() << ","
            << table_->item(i, 1)->text() << ","
            << table_->item(i, 2)->text().replace("s", "") << ","
            << table_->item(i, 3)->text().replace("%", "") << "\n";
    }

    file.close();
    return true;
}
