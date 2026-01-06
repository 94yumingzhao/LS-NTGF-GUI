// results_widget.h - Dynamic Results Display Widget
// Supports different display modes for RF/RFO/RR algorithms

#ifndef RESULTS_WIDGET_H_
#define RESULTS_WIDGET_H_

#include <QGroupBox>
#include <QString>
#include "solver_worker.h"  // For AlgorithmType

class QTableWidget;
class QLabel;

class ResultsWidget : public QGroupBox {
    Q_OBJECT

public:
    explicit ResultsWidget(QWidget* parent = nullptr);

    // Set algorithm type to change display mode
    void SetAlgorithmType(AlgorithmType algo);

    // Clear all results
    void ClearResults();

    // Set merge info (original -> merged)
    void SetMergeInfo(int original, int merged);

    // Set merge skipped (no merge performed)
    void SetMergeSkipped();

    // Set stage result (stage index depends on algorithm)
    // RR: stage 1-3, RF: stage 0 (summary), RFO: stage 1 (RF), 2 (FO)
    void SetStageResult(int stage, double objective, double runtime, double gap);

    // Set total runtime
    void SetTotalRuntime(double runtime);

    bool HasResults() const;

private:
    void SetupUi();
    void SetupTableForRR();
    void SetupTableForRF();
    void SetupTableForRFO();
    void ClearTable();

    AlgorithmType current_algo_;
    QTableWidget* table_;
    QLabel* merge_label_;
    QLabel* total_label_;
    bool has_results_;
};

#endif  // RESULTS_WIDGET_H_
