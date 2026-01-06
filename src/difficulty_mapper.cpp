// difficulty_mapper.cpp - Difficulty Level to Parameter Mapping Implementation

#include "difficulty_mapper.h"

// Static member definitions
constexpr double DifficultyMapper::kCapacityUtil[];
constexpr int DifficultyMapper::kTimeWindowOffset[];
constexpr double DifficultyMapper::kDemandCV[];
constexpr double DifficultyMapper::kPeakRatio[];
constexpr double DifficultyMapper::kPeakMultiplier[];
constexpr int DifficultyMapper::kZoom[];

GeneratorConfig DifficultyMapper::GetPreset(DifficultyLevel difficulty, ScaleLevel scale) {
    GeneratorConfig config;

    int d = static_cast<int>(difficulty);

    // Scale-based order count
    switch (scale) {
        case ScaleLevel::Small:
            config.N = (d == 0) ? 50 : (d == 1) ? 80 : 100;
            break;
        case ScaleLevel::Medium:
            config.N = (d == 0) ? 150 : (d == 1) ? 200 : 300;
            break;
        case ScaleLevel::Large:
            config.N = (d == 0) ? 500 : (d == 1) ? 700 : 1000;
            break;
    }

    // Fixed dimensions
    config.T = 30;
    config.F = 5;
    config.G = 5;

    // Difficulty-based parameters
    config.capacity_utilization = kCapacityUtil[d];
    config.time_window_offset = kTimeWindowOffset[d];
    config.demand_cv = kDemandCV[d];
    config.peak_ratio = kPeakRatio[d];
    config.peak_multiplier = kPeakMultiplier[d];
    config.zoom = kZoom[d];

    // Time window differentiation
    config.urgent_ratio = 0.10;
    config.flexible_ratio = 0.20;

    // Cost correlation enabled for higher difficulty
    config.cost_correlation = (d >= 1);

    // Defaults
    config.seed = 0;
    config.count = 1;
    config.output_path = "D:/YM-Code/LS-NTGF-Data-Cap/data/";

    return config;
}

QString DifficultyMapper::GetDifficultyName(DifficultyLevel level) {
    switch (level) {
        case DifficultyLevel::Easy:   return "Easy";
        case DifficultyLevel::Medium: return "Medium";
        case DifficultyLevel::Hard:   return "Hard";
        case DifficultyLevel::Expert: return "Expert";
        default: return "Unknown";
    }
}

QString DifficultyMapper::GetScaleName(ScaleLevel level) {
    switch (level) {
        case ScaleLevel::Small:  return "Small (50-100)";
        case ScaleLevel::Medium: return "Medium (150-300)";
        case ScaleLevel::Large:  return "Large (500-1000)";
        default: return "Unknown";
    }
}

QString DifficultyMapper::EstimateGap(const GeneratorConfig& config) {
    double score = EstimateDifficultyScore(config);

    if (score < 0.5) return "<2%";
    if (score < 1.0) return "2-5%";
    if (score < 1.5) return "5-15%";
    if (score < 2.0) return "15-30%";
    return ">30%";
}

double DifficultyMapper::EstimateDifficultyScore(const GeneratorConfig& config) {
    // Difficulty scoring formula from analysis document
    double score = 0.0;

    // Capacity utilization (weight: 0.30)
    score += 0.30 * (config.capacity_utilization / 0.70);

    // Time window tightness (weight: 0.20)
    double avg_window = 2.0 * config.time_window_offset + 1.0;
    score += 0.20 * (1.0 - avg_window / config.T);

    // Problem scale (weight: 0.20)
    score += 0.20 * (static_cast<double>(config.N) * config.T / 3000.0);

    // Demand variability (weight: 0.15)
    score += 0.15 * (config.peak_multiplier / 2.0);

    // Group complexity (weight: 0.15)
    score += 0.15 * (static_cast<double>(config.G) / 5.0);

    return score;
}
