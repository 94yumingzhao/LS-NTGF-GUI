// overview_panel.cpp - Overview panel implementation

#include "overview_panel.h"
#include "../widgets/metric_card.h"
#include "../widgets/cost_bar.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QJsonObject>
#include <QJsonArray>

OverviewPanel::OverviewPanel(QWidget* parent)
    : QWidget(parent) {
    SetupUi();
}

void OverviewPanel::SetupUi() {
    auto* layout = new QVBoxLayout(this);
    layout->setSpacing(12);
    layout->setContentsMargins(8, 8, 8, 8);

    // Metric cards grid (2 rows x 3 cols)
    auto* cards_layout = new QGridLayout();
    cards_layout->setSpacing(8);

    card_objective_ = new MetricCard(QString::fromUtf8("目标值"), this);
    card_time_ = new MetricCard(QString::fromUtf8("求解时间"), this);
    card_gap_ = new MetricCard(QString::fromUtf8("Gap"), this);
    card_unmet_ = new MetricCard(QString::fromUtf8("未满足"), this);
    card_setups_ = new MetricCard(QString::fromUtf8("Setup"), this);
    card_carryovers_ = new MetricCard(QString::fromUtf8("Carryover"), this);

    cards_layout->addWidget(card_objective_, 0, 0);
    cards_layout->addWidget(card_time_, 0, 1);
    cards_layout->addWidget(card_gap_, 0, 2);
    cards_layout->addWidget(card_unmet_, 1, 0);
    cards_layout->addWidget(card_setups_, 1, 1);
    cards_layout->addWidget(card_carryovers_, 1, 2);

    layout->addLayout(cards_layout);

    // Cost breakdown
    cost_group_ = new QGroupBox(QString::fromUtf8("成本构成"), this);
    auto* cost_layout = new QVBoxLayout(cost_group_);
    cost_bar_ = new CostBar(this);
    cost_layout->addWidget(cost_bar_);
    layout->addWidget(cost_group_);

    // Problem info
    problem_group_ = new QGroupBox(QString::fromUtf8("问题规模"), this);
    auto* problem_layout = new QVBoxLayout(problem_group_);
    problem_label_ = new QLabel("--", this);
    problem_label_->setStyleSheet("font-family: monospace;");
    problem_layout->addWidget(problem_label_);
    layout->addWidget(problem_group_);

    // Algorithm-specific metrics
    algo_group_ = new QGroupBox(QString::fromUtf8("算法指标"), this);
    auto* algo_layout = new QVBoxLayout(algo_group_);
    algo_label_ = new QLabel("--", this);
    algo_label_->setStyleSheet("font-family: monospace; font-size: 9pt;");
    algo_label_->setWordWrap(true);
    algo_layout->addWidget(algo_label_);
    layout->addWidget(algo_group_);

    layout->addStretch();
}

void OverviewPanel::LoadData(const QJsonObject& json) {
    // Summary section
    QJsonObject summary = json["summary"].toObject();
    QString algo = summary["algorithm"].toString();

    double objective = summary["objective"].toDouble();
    double solve_time = summary["solve_time"].toDouble();
    double gap = summary["gap"].toDouble();
    int unmet_count = summary["unmet_count"].toInt();

    card_objective_->SetValue(objective, 2);
    card_time_->SetValue(QString("%1s").arg(solve_time, 0, 'f', 2));
    card_gap_->SetValue(QString("%1%").arg(gap * 100, 0, 'f', 2));
    card_unmet_->SetValue(unmet_count);

    // Metrics section
    QJsonObject metrics = json["metrics"].toObject();

    // Setup/Carryover
    QJsonObject setup_carryover = metrics["setup_carryover"].toObject();
    int total_setups = setup_carryover["total_setups"].toInt();
    int total_carryovers = setup_carryover["total_carryovers"].toInt();
    card_setups_->SetValue(total_setups);
    card_carryovers_->SetValue(total_carryovers);

    // Cost breakdown
    QJsonObject cost = metrics["cost"].toObject();
    double cost_production = cost["production"].toDouble();
    double cost_setup = cost["setup"].toDouble();
    double cost_inventory = cost["inventory"].toDouble();
    double cost_backorder = cost["backorder"].toDouble();
    double cost_unmet = cost["unmet"].toDouble();
    cost_bar_->SetCosts(cost_production, cost_setup, cost_inventory,
                        cost_backorder, cost_unmet);

    // Problem info
    QJsonObject problem = json["problem"].toObject();
    int N = problem["N"].toInt();
    int T = problem["T"].toInt();
    int G = problem["G"].toInt();
    int F = problem["F"].toInt();
    int cap = problem["capacity"].toInt();
    problem_label_->setText(
        QString("N=%1  T=%2  G=%3  F=%4  Capacity=%5")
            .arg(N).arg(T).arg(G).arg(F).arg(cap));

    // Algorithm-specific metrics
    QJsonObject algo_specific = metrics["algorithm_specific"].toObject();
    QString algo_text;

    if (algo == "RF") {
        algo_group_->setTitle(QString::fromUtf8("RF 算法指标"));
        algo_text = QString::fromUtf8(
            "迭代次数: %1    窗口扩展: %2    回滚: %3\n"
            "子问题数: %4    平均子问题时间: %5s\n"
            "最终求解时间: %6s")
            .arg(algo_specific["rf_iterations"].toInt())
            .arg(algo_specific["rf_window_expansions"].toInt())
            .arg(algo_specific["rf_rollbacks"].toInt())
            .arg(algo_specific["rf_subproblems"].toInt())
            .arg(algo_specific["rf_avg_subproblem_time"].toDouble(), 0, 'f', 3)
            .arg(algo_specific["rf_final_solve_time"].toDouble(), 0, 'f', 3);
    } else if (algo == "RFO") {
        algo_group_->setTitle(QString::fromUtf8("RFO 算法指标"));
        algo_text = QString::fromUtf8(
            "RF阶段: 目标=%1  时间=%2s\n"
            "FO阶段: 轮数=%3  改进窗口=%4\n"
            "FO改进: %5 (%6%)\n"
            "FO时间: %7s    最终求解时间: %8s")
            .arg(algo_specific["rfo_rf_objective"].toDouble(), 0, 'f', 2)
            .arg(algo_specific["rfo_rf_time"].toDouble(), 0, 'f', 3)
            .arg(algo_specific["rfo_fo_rounds"].toInt())
            .arg(algo_specific["rfo_fo_windows_improved"].toInt())
            .arg(algo_specific["rfo_fo_improvement"].toDouble(), 0, 'f', 2)
            .arg(algo_specific["rfo_fo_improvement_pct"].toDouble() * 100, 0, 'f', 2)
            .arg(algo_specific["rfo_fo_time"].toDouble(), 0, 'f', 3)
            .arg(algo_specific["rfo_final_solve_time"].toDouble(), 0, 'f', 3);
    } else if (algo == "RR") {
        algo_group_->setTitle(QString::fromUtf8("RR (PP-GCB) 算法指标"));
        algo_text = QString::fromUtf8(
            "Step1: 目标=%1  Setup=%2  时间=%3s\n"
            "Step2: Carryover=%4  时间=%5s\n"
            "Step3: 目标=%6  时间=%7s\n"
            "Step3-Step1 Gap: %8%    Carryover利用率: %9%")
            .arg(algo_specific["rr_step1_objective"].toDouble(), 0, 'f', 2)
            .arg(algo_specific["rr_step1_setups"].toInt())
            .arg(algo_specific["rr_step1_time"].toDouble(), 0, 'f', 3)
            .arg(algo_specific["rr_step2_carryovers"].toInt())
            .arg(algo_specific["rr_step2_time"].toDouble(), 0, 'f', 3)
            .arg(algo_specific["rr_step3_objective"].toDouble(), 0, 'f', 2)
            .arg(algo_specific["rr_step3_time"].toDouble(), 0, 'f', 3)
            .arg(algo_specific["rr_step3_gap_to_step1"].toDouble() * 100, 0, 'f', 2)
            .arg(algo_specific["rr_carryover_utilization"].toDouble() * 100, 0, 'f', 1);
    } else {
        algo_group_->setTitle(QString::fromUtf8("算法指标"));
        algo_text = QString::fromUtf8("算法: %1").arg(algo);
    }

    algo_label_->setText(algo_text);
}

void OverviewPanel::Clear() {
    card_objective_->SetValue("--");
    card_time_->SetValue("--");
    card_gap_->SetValue("--");
    card_unmet_->SetValue("--");
    card_setups_->SetValue("--");
    card_carryovers_->SetValue("--");
    cost_bar_->Clear();
    problem_label_->setText("--");
    algo_group_->setTitle(QString::fromUtf8("算法指标"));
    algo_label_->setText("--");
}
