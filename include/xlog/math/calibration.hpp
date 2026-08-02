#ifndef XLOG_MATH_CALIBRATION_HPP
#define XLOG_MATH_CALIBRATION_HPP

namespace xlog {
namespace math {

struct RecalibrationResult {
    double cr_new;
    double d_new;
};

RecalibrationResult recalibrateDifficulty(double cr_old, double d_old, double d_original, bool completed_on_schedule);
double calculatePriority(double priority_base, int days_overdue, double balance_val, int days_stale);

} // namespace math
} // namespace xlog

#endif // XLOG_MATH_CALIBRATION_HPP
