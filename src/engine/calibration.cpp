#include "xlog/math/calibration.hpp"
#include <algorithm>
#include <cmath>

namespace xlog {
namespace math {

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
