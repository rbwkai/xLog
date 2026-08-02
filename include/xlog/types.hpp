#ifndef XLOG_TYPES_HPP
#define XLOG_TYPES_HPP

#include <string>
#include <vector>
#include <optional>
#include <cstdint>

namespace xlog {

enum class TaskType {
    Hobby,
    OneTime,
    Periodic,
    Recurring
};

inline std::string taskTypeToString(TaskType type) {
    switch (type) {
        case TaskType::Hobby: return "hobby";
        case TaskType::OneTime: return "one_time";
        case TaskType::Periodic: return "periodic";
        case TaskType::Recurring: return "recurring";
    }
    return "one_time";
}

inline TaskType stringToTaskType(const std::string& str) {
    if (str == "hobby") return TaskType::Hobby;
    if (str == "periodic") return TaskType::Periodic;
    if (str == "recurring") return TaskType::Recurring;
    return TaskType::OneTime;
}

enum class Rank {
    Gray,
    Green,
    Cyan,
    Blue,
    Violet,
    Orange,
    Red
};

inline std::string rankToString(Rank rank) {
    switch (rank) {
        case Rank::Gray: return "Gray";
        case Rank::Green: return "Green";
        case Rank::Cyan: return "Cyan";
        case Rank::Blue: return "Blue";
        case Rank::Violet: return "Violet";
        case Rank::Orange: return "Orange";
        case Rank::Red: return "Red";
    }
    return "Gray";
}

inline Rank stringToRank(const std::string& str) {
    if (str == "Green") return Rank::Green;
    if (str == "Cyan") return Rank::Cyan;
    if (str == "Blue") return Rank::Blue;
    if (str == "Violet") return Rank::Violet;
    if (str == "Orange") return Rank::Orange;
    if (str == "Red") return Rank::Red;
    return Rank::Gray;
}

struct User {
    int64_t id{0};
    std::string username;
    std::string timezone{"UTC"};
    int day_boundary_hour{4};
    double rating_current{1.0};
    std::string rank_current{"Gray"};
    double debt_current{0.0};
    int grace_tokens{0};
    int streak_days{0};
    int longest_streak{0};
    double daily_budget_ema{0.0};
    int crit_pity_k{0};
    std::string last_active_date;
};

struct Domain {
    int64_t id{0};
    int64_t user_id{0};
    std::string name;
    int order_index{0};
    std::string color_code;
    double score_cached{1.0};
};

struct Subdomain {
    int64_t id{0};
    int64_t domain_id{0};
    std::string name;
    int order_index{0};
    double xp_raw_total{0.0};
    double xp_eff_cached{0.0};
    double score_cached{0.0};
    std::string last_activity_date;
};

struct Task {
    int64_t id{0};
    int64_t user_id{0};
    std::string name;
    TaskType type{TaskType::OneTime};
    int64_t major_subdomain_id{0};
    int64_t minor_subdomain_id{0};
    double difficulty_current{15.0};
    double difficulty_original{15.0};
    double cr_ema{0.775};
    double priority_base{0.5};
    double priority_current{0.5};
    std::optional<std::string> started_at_date;
    std::optional<int> period_days;
    std::optional<int> recurrence_mask;
    bool due_today{false};
    std::optional<std::string> first_appeared_date;
    std::optional<std::string> reward_unlock;
    std::string status{"active"};
    std::optional<std::string> last_completed_date;
    int total_completions{0};
    double total_xp_earned{0.0};
};

struct TaskCompletion {
    int64_t id{0};
    int64_t task_id{0};
    int64_t completed_at{0};
    std::string local_date;
    double t_actual{0.0};
    double difficulty_at_completion{0.0};
    double xp_raw{0.0};
    double m_frog{1.0};
    double m_crit{1.0};
    double m_streak{1.0};
    double m_balance{1.0};
    double xp_final{0.0};
    double xp_major{0.0};
    double xp_minor{0.0};
    bool was_frog{false};
    bool was_crit{false};
    double debt_redeemed{0.0};
    double cr_after{0.0};
};

struct Quote {
    int64_t id{0};
    int64_t user_id{0};
    std::string text;
    std::string author;
    bool active{true};
    int times_shown{0};
    int64_t last_shown_at{0};
};

} // namespace xlog

#endif // XLOG_TYPES_HPP
