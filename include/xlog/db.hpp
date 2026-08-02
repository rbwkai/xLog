#ifndef XLOG_DB_HPP
#define XLOG_DB_HPP

#include "xlog/types.hpp"
#include <sqlite3.h>
#include <string>
#include <vector>
#include <optional>
#include <memory>

namespace xlog {

class Database {
public:
    explicit Database(const std::string& db_path);
    ~Database();

    // Disable copy
    Database(const Database&) = delete;
    Database& operator=(const Database&) = delete;

    void initializeSchema();

    // User Operations
    std::optional<User> getUser();
    User onboardUser(const std::string& username, const std::vector<std::pair<std::string, std::vector<std::string>>>& domain_structure);
    void updateUser(const User& user);

    // Domains & Subdomains
    std::vector<Domain> getDomains(int64_t user_id);
    std::vector<Subdomain> getSubdomains(int64_t domain_id);
    std::vector<Subdomain> getAllSubdomains(int64_t user_id);
    std::optional<Subdomain> getSubdomainById(int64_t subdomain_id);
    std::optional<Domain> getDomainById(int64_t domain_id);

    // Tasks
    int64_t createTask(const Task& task);
    std::optional<Task> getTaskById(int64_t task_id);
    std::optional<Task> getTaskByName(int64_t user_id, const std::string& name);
    std::vector<Task> getAllTasks(int64_t user_id);
    std::vector<Task> getTodayTasks(int64_t user_id);
    std::vector<Task> getBoredTasks(int64_t user_id);
    std::vector<Task> getQuickTasks(int64_t user_id);
    void updateTask(const Task& task);

    // Completions & Engine Pipeline
    void recordCompletion(TaskCompletion& completion, Task& task, User& user);

    // Daily Rollover
    void checkAndRunDailyRollover(User& user, const std::string& today_date);

    // Quotes
    std::optional<Quote> getRandomQuote(int64_t user_id);
    int64_t addQuote(int64_t user_id, const std::string& text, const std::string& author);
    bool deleteQuote(int64_t quote_id);
    std::vector<Quote> getAllQuotes(int64_t user_id);

    // Stats / Profile
    struct ProfileStats {
        User user;
        std::vector<std::pair<Domain, std::vector<Subdomain>>> domains;
        int total_tasks_completed{0};
        double total_xp_earned{0.0};
        std::vector<std::pair<std::string, double>> last_30_days_xp;
    };
    ProfileStats getProfileStats(int64_t user_id);

private:
    sqlite3* db_{nullptr};
    void execSql(const std::string& sql);
};

} // namespace xlog

#endif // XLOG_DB_HPP
