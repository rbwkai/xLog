#ifndef XLOG_MATH_SCORING_HPP
#define XLOG_MATH_SCORING_HPP

#include "xlog/types.hpp"
#include <vector>

namespace xlog {
namespace math {

double calculateSubdomainScore(double xp_eff);
double calculateDomainScore(const std::vector<double>& subdomain_scores);
double calculateRating(const std::vector<double>& domain_scores);
Rank ratingToRank(double rating);
double pointsToNextRank(double rating);
double calculateSubdomainDecay(double xp_raw_total, int days_inactive);

} // namespace math
} // namespace xlog

#endif // XLOG_MATH_SCORING_HPP
