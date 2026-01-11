// results_widget.cpp - Dynamic Results Display Widget Implementation

#include "results_widget.h"

#include <QVBoxLayout>
#include <QTableWidget>
#include <QHeaderView>
#include <QLabel>

ResultsWidget::ResultsWidget(QWidget* parent)
    : QGroupBox(QString::fromUtf8("结果"), parent)
    , current_algo_(AlgorithmType::RF)
    , has_results_(false) {
    SetupUi();
}

void ResultsWidget::SetupUi() {
    auto* layout = new QVBoxLayout(this);
    layout->setSpacing(4);

    // Merge info label
    merge_label_ = new QLabel("--", this);
    merge_label_->setStyleSheet("color: gray; font-size: 9pt;");
    layout->addWidget(merge_label_);

    // Results table
    table_ = new QTableWidget(this);
    table_->verticalHeader()->setVisible(false);
    table_->horizontalHeader()->setStretchLastSection(true);
    table_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table_->setSelectionBehavior(QAbstractItemView::SelectRows);
    table_->setMaximumHeight(150);
    layout->addWidget(table_);

    // Total runtime label
    total_label_ = new QLabel(QString::fromUtf8("总耗时: --"), this);
    total_label_->setStyleSheet("font-weight: bold;");
    layout->addWidget(total_label_);

    // Initialize with default algorithm
    SetupTableForRF();
}

void ResultsWidget::SetAlgorithmType(AlgorithmType algo) {
    if (current_algo_ == algo) return;

    current_algo_ = algo;
    ClearResults();

    switch (algo) {
        case AlgorithmType::RF:
            SetupTableForRF();
            break;
        case AlgorithmType::RFO:
            SetupTableForRFO();
            break;
        case AlgorithmType::RR:
            SetupTableForRR();
            break;
    }
}

void ResultsWidget::SetupTableForRR() {
    // RR: 3 stages (Setup, Carryover, Final)
    table_->clear();
    table_->setRowCount(3);
    table_->setColumnCount(4);
    table_->setHorizontalHeaderLabels({
        QString::fromUtf8("阶段"),
        QString::fromUtf8("目标值"),
        QString::fromUtf8("耗时"),
        "Gap"
    });

    table_->setColumnWidth(0, 70);
    table_->setColumnWidth(1, 90);
    table_->setColumnWidth(2, 50);
    table_->setColumnWidth(3, 50);

    QStringList stages = {
        QString::fromUtf8("初始"),
        QString::fromUtf8("跨期"),
        QString::fromUtf8("最终")
    };
    for (int i = 0; i < 3; ++i) {
        table_->setItem(i, 0, new QTableWidgetItem(stages[i]));
        table_->setItem(i, 1, new QTableWidgetItem("--"));
        table_->setItem(i, 2, new QTableWidgetItem("--"));
        table_->setItem(i, 3, new QTableWidgetItem("--"));
    }
}

void ResultsWidget::SetupTableForRF() {
    // RF: Summary only (windows shown in log)
    table_->clear();
    table_->setRowCount(1);
    table_->setColumnCount(3);
    table_->setHorizontalHeaderLabels({
        "",
        QString::fromUtf8("目标值"),
        QString::fromUtf8("耗时")
    });

    table_->setColumnWidth(0, 70);
    table_->setColumnWidth(1, 90);
    table_->setColumnWidth(2, 50);

    table_->setItem(0, 0, new QTableWidgetItem(QString::fromUtf8("结果")));
    table_->setItem(0, 1, new QTableWidgetItem("--"));
    table_->setItem(0, 2, new QTableWidgetItem("--"));
}

void ResultsWidget::SetupTableForRFO() {
    // RFO: 2 phases (RF, Fix-Optimize)
    table_->clear();
    table_->setRowCount(2);
    table_->setColumnCount(4);
    table_->setHorizontalHeaderLabels({
        QString::fromUtf8("阶段"),
        QString::fromUtf8("目标值"),
        QString::fromUtf8("耗时"),
        QString::fromUtf8("改进")
    });

    table_->setColumnWidth(0, 70);
    table_->setColumnWidth(1, 90);
    table_->setColumnWidth(2, 50);
    table_->setColumnWidth(3, 50);

    table_->setItem(0, 0, new QTableWidgetItem("RF"));
    table_->setItem(0, 1, new QTableWidgetItem("--"));
    table_->setItem(0, 2, new QTableWidgetItem("--"));
    table_->setItem(0, 3, new QTableWidgetItem("--"));

    table_->setItem(1, 0, new QTableWidgetItem("FO"));
    table_->setItem(1, 1, new QTableWidgetItem("--"));
    table_->setItem(1, 2, new QTableWidgetItem("--"));
    table_->setItem(1, 3, new QTableWidgetItem("--"));
}

void ResultsWidget::ClearResults() {
    ClearTable();
    merge_label_->setText("--");
    total_label_->setText(QString::fromUtf8("总耗时: --"));
    has_results_ = false;
}

void ResultsWidget::ClearTable() {
    int rows = table_->rowCount();
    int cols = table_->columnCount();
    for (int r = 0; r < rows; ++r) {
        for (int c = 1; c < cols; ++c) {  // Skip first column (stage name)
            if (table_->item(r, c)) {
                table_->item(r, c)->setText("--");
            }
        }
    }
}

void ResultsWidget::SetMergeInfo(int original, int merged) {
    merge_label_->setText(QString::fromUtf8("合并: %1 -> %2").arg(original).arg(merged));
    merge_label_->setStyleSheet("color: black; font-size: 9pt;");
}

void ResultsWidget::SetMergeSkipped() {
    merge_label_->setText(QString::fromUtf8("合并: 已跳过"));
    merge_label_->setStyleSheet("color: gray; font-size: 9pt;");
}

void ResultsWidget::SetStageResult(int stage, double objective, double runtime, double gap) {
    int row = -1;

    switch (current_algo_) {
        case AlgorithmType::RR:
            // RR: stage 1-3 maps to row 0-2
            if (stage >= 1 && stage <= 3) {
                row = stage - 1;
            }
            break;
        case AlgorithmType::RF:
            // RF: any stage maps to row 0 (summary)
            row = 0;
            break;
        case AlgorithmType::RFO:
            // RFO: stage 1 = RF phase (row 0), stage 2 = FO phase (row 1)
            if (stage >= 1 && stage <= 2) {
                row = stage - 1;
            }
            break;
    }

    if (row >= 0 && row < table_->rowCount()) {
        table_->item(row, 1)->setText(QString::number(objective, 'f', 2));
        table_->item(row, 2)->setText(QString("%1s").arg(runtime, 0, 'f', 1));

        // Gap column (RR and RFO have it, RF doesn't)
        if (table_->columnCount() > 3 && table_->item(row, 3)) {
            if (current_algo_ == AlgorithmType::RR) {
                table_->item(row, 3)->setText(QString("%1%").arg(gap * 100, 0, 'f', 2));
            } else if (current_algo_ == AlgorithmType::RFO) {
                // For RFO, column 3 is improvement percentage
                table_->item(row, 3)->setText(QString("%1%").arg(gap * 100, 0, 'f', 2));
            }
        }
        has_results_ = true;
    }
}

void ResultsWidget::SetTotalRuntime(double runtime) {
    total_label_->setText(QString::fromUtf8("总耗时: %1s").arg(runtime, 0, 'f', 2));
}

bool ResultsWidget::HasResults() const {
    return has_results_;
}
