// difficulty_mapper.h - Difficulty Level to Parameter Mapping
//
// Maps difficulty presets (Easy/Medium/Hard/Expert) to generator parameters

#ifndef DIFFICULTY_MAPPER_H_
#define DIFFICULTY_MAPPER_H_

#include <QString>

// Difficulty preset levels
enum class DifficultyLevel {
    Easy = 0,
    Medium = 1,
    Hard = 2,
    Expert = 3
};

// Problem scale levels
enum class ScaleLevel {
    Small = 0,   // N=50-100
    Medium = 1,  // N=100-300
    Large = 2    // N=500-1000
};

// Generator configuration parameters
struct GeneratorConfig {
    // Problem scale
    int N;  // Order count
    int T;  // Time periods
    int F;  // Flow count
    int G;  // Group count

    // Difficulty parameters
    double capacity_utilization;  // 0.50-0.95
    int time_window_offset;       // 3-10
    double demand_cv;             // 0.0-0.5
    double peak_ratio;            // 0.0-0.3
    double peak_multiplier;       // 1.5-3.0
    double urgent_ratio;          // 0.0-0.3
    double flexible_ratio;        // 0.0-0.3
    bool cost_correlation;        // true/false

    // Other parameters
    int zoom;       // Capacity scaling factor
    int seed;       // Random seed (0 = auto)
    int count;      // Number of instances to generate
    QString output_path;  // Output directory
};

// Difficulty mapper utility class
class DifficultyMapper {
public:
    // Get preset configuration for a difficulty level
    static GeneratorConfig GetPreset(DifficultyLevel difficulty, ScaleLevel scale);

    // Get difficulty name string
    static QString GetDifficultyName(DifficultyLevel level);

    // Get scale name string
    static QString GetScaleName(ScaleLevel level);

    // Estimate MIP gap for given configuration
    static QString EstimateGap(const GeneratorConfig& config);

    // Estimate difficulty score (0.0-2.5)
    static double EstimateDifficultyScore(const GeneratorConfig& config);

private:
    // Base parameter values for each difficulty level
    static constexpr double kCapacityUtil[] = {0.55, 0.70, 0.85, 0.95};
    static constexpr int kTimeWindowOffset[] = {8, 5, 4, 3};
    static constexpr double kDemandCV[] = {0.15, 0.25, 0.35, 0.45};
    static constexpr double kPeakRatio[] = {0.10, 0.15, 0.20, 0.25};
    static constexpr double kPeakMultiplier[] = {1.5, 2.0, 2.5, 3.0};
    static constexpr int kZoom[] = {70, 60, 50, 45};
};

#endif  // DIFFICULTY_MAPPER_H_
