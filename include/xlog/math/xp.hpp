#ifndef XLOG_MATH_XP_HPP
#define XLOG_MATH_XP_HPP

namespace xlog {
namespace math {

double calculateRawXp(double t_actual, double difficulty_current);
double rollFrogMultiplier();
double rollCritMultiplier(int& crit_pity_k, bool& was_crit);
double calculateStreakMultiplier(int streak_days);
double calculateBalance(double domain_score, double rating);
double calculateBalanceBonus(double domain_score, double rating);

} // namespace math
} // namespace xlog

#endif // XLOG_MATH_XP_HPP
