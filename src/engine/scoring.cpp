#include "xlog/math/scoring.hpp"
#include <algorithm>
#include <cmath>

namespace xlog {
namespace math {

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

} // namespace math
} // namespace xlog
