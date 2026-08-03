#include "xlog/math/xp.hpp"
#include <algorithm>
#include <cmath>
#include <random>

namespace xlog {
namespace math {

static std::mt19937& getRng() {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    return gen;
}

double calculateRawXp(double t_actual, double difficulty_current) {
    if (difficulty_current <= 0) return 0.0;
    return std::clamp(t_actual, 0.0, 3.0 * difficulty_current);
}

double rollFrogMultiplier() {
    std::uniform_real_distribution<double> dist(0.0, 1.0);
    double u = dist(getRng());
    double min_val = 1.5;
    double mode_val = 2.5;
    double max_val = 4.0;
    double threshold = (mode_val - min_val) / (max_val - min_val);

    if (u < threshold) {
        return min_val + std::sqrt(u * (max_val - min_val) * (mode_val - min_val));
    } else {
        return max_val - std::sqrt((1.0 - u) * (max_val - min_val) * (max_val - mode_val));
    }
}

double rollCritMultiplier(int& crit_pity_k, bool& was_crit) {
    double p_base = 0.08;
    int m_soft = 15;
    int n_hard = 30;
    double p = p_base;

    if (crit_pity_k >= n_hard) {
        p = 1.0;
    } else if (crit_pity_k >= m_soft) {
        p = p_base + static_cast<double>(crit_pity_k - m_soft) / (n_hard - m_soft) * (1.0 - p_base);
    }

    std::uniform_real_distribution<double> dist(0.0, 1.0);
    double u = dist(getRng());

    if (u < p) {
        was_crit = true;
        crit_pity_k = 0;
        std::uniform_real_distribution<double> crit_dist(1.3, 1.8);
        return crit_dist(getRng());
    } else {
        was_crit = false;
        crit_pity_k++;
        return 1.0;
    }
}

double calculateStreakMultiplier(int streak_days) {
    if (streak_days <= 0) return 1.0;
    return 1.0 + 0.05 * std::log(1.0 + static_cast<double>(streak_days) / 30.0);
}

double calculateBalance(double domain_score, double rating) {
    if (rating <= 0.0) return 0.0;
    return std::max(0.0, (rating - domain_score) / rating);
}

double calculateBalanceBonus(double domain_score, double rating) {
    return std::min(1.4, 1.0 + 0.5 * calculateBalance(domain_score, rating));
}

} // namespace math
} // namespace xlog
