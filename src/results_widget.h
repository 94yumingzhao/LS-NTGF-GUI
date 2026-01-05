// results_widget.h - Results Display Widget

#ifndef RESULTS_WIDGET_H_
#define RESULTS_WIDGET_H_

#include <QGroupBox>
#include <QString>

class QTableWidget;

class ResultsWidget : public QGroupBox {
    Q_OBJECT

public:
    explicit ResultsWidget(QWidget* parent = nullptr);

    void ClearResults();
    void SetStageResult(int row, double objective, double runtime, double gap);
    bool HasResults() const;
    bool ExportToCsv(const QString& path) const;

private:
    void SetupUi();

    QTableWidget* table_;
    bool has_results_;
};

#endif  // RESULTS_WIDGET_H_
