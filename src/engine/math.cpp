#include "xlog/math.hpp"
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

double calculateBalanceBonus(double domain_score, double rating) {
    if (rating <= 0.0) return 1.0;
    double balance = std::max(0.0, (rating - domain_score) / rating);
    return std::min(1.4, 1.0 + 0.5 * balance);
}

double calculateSubdomainScore(double xp_eff) {
    if (xp_eff <= 0.0) return 0.0;
    return 850.0 * std::log(1.0 + xp_eff / 1000.0);
}

double calculateDomainScore(const std::vector<double>& subdomain_scores) {
    if (subdomain_scores.empty()) return 1.0;
    double sum = 0.0;
    for (double s : subdomain_scores) {
        sum += s;
    }
    double mean = sum / subdomain_scores.size();
    return std::max(1.0, mean);
}

double calculateRating(const std::vector<double>& domain_scores) {
    if (domain_scores.empty()) return 1.0;
    double sum_inv = 0.0;
    for (double ds : domain_scores) {
        double floored_ds = std::max(1.0, ds);
        sum_inv += (1.0 / floored_ds);
    }
    if (sum_inv <= 0.0) return 1.0;
    return domain_scores.size() / sum_inv;
}

Rank ratingToRank(double rating) {
    if (rating >= 3000.0) return Rank::Red;
    if (rating >= 2500.0) return Rank::Orange;
    if (rating >= 2000.0) return Rank::Violet;
    if (rating >= 1500.0) return Rank::Blue;
    if (rating >= 1000.0) return Rank::Cyan;
    if (rating >= 500.0) return Rank::Green;
    return Rank::Gray;
}

double pointsToNextRank(double rating) {
    if (rating < 500.0) return 500.0 - rating;
    if (rating < 1000.0) return 1000.0 - rating;
    if (rating < 1500.0) return 1500.0 - rating;
    if (rating < 2000.0) return 2000.0 - rating;
    if (rating < 2500.0) return 2500.0 - rating;
    if (rating < 3000.0) return 3000.0 - rating;
    return 0.0;
}

double calculateSubdomainDecay(double xp_raw_total, int days_inactive) {
    if (days_inactive <= 0) return xp_raw_total;
    double r_t = std::max(0.70, std::exp(-static_cast<double>(days_inactive) / 90.0));
    return xp_raw_total * r_t;
}

RecalibrationResult recalibrateDifficulty(double cr_old, double d_old, double d_original, bool completed_on_schedule) {
    double alpha = 2.0 / 11.0; // span = 10
    double c_t = completed_on_schedule ? 1.0 : 0.0;
    double cr_new = alpha * c_t + (1.0 - alpha) * cr_old;

    double d_new = d_old;
    double cr_low = 0.70;
    double cr_high = 0.85;
    double k_up = 0.15;
    double k_down = 0.20;

    if (cr_new > cr_high) {
        d_new = d_old * (1.0 + k_up * (cr_new - cr_high));
    } else if (cr_new < cr_low) {
        d_new = d_old * (1.0 - k_down * (cr_low - cr_new));
    }

    double min_guardrail = std::max(5.0, 0.3 * d_original);
    double max_guardrail = 3.0 * d_original;
    d_new = std::clamp(d_new, min_guardrail, max_guardrail);

    return {cr_new, d_new};
}

double calculatePriority(double priority_base, int days_overdue, double balance_val, int days_stale) {
    double w1 = 0.40;
    double w2 = 0.30;
    double w3 = 0.20;
    double w4 = 0.10;

    double u_debt = (days_overdue > 0) ? (1.0 - std::exp(-0.6 * days_overdue)) : 0.0;
    double n_d = std::min(1.0, 1.2 * balance_val);
    double s_stale = (days_stale > 0) ? (1.0 - std::exp(-0.4 * days_stale)) : 0.0;

    double p = w1 * priority_base + w2 * u_debt + w3 * n_d + w4 * s_stale;
    return std::clamp(p, 0.0, 1.0);
}

} // namespace math
} // namespace xlog
