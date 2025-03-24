// DepositManager.h - Handles deposit creation and management with Redis
#ifndef DEPOSIT_MANAGER_H
#define DEPOSIT_MANAGER_H

#include <string>
#include <vector>
#include <map>
#include <mutex>
#include "AccountManager.h"
#include "RedisClient.h"

class DepositManager {
private:
    AccountManager& accountManager;
    RedisClient redis;
    std::mutex counterMutex; // 保护计数器访问的互斥锁

    // 为指定用户生成下一个存款ID
    std::string generateNextDepositId(const std::string& username);

    // 从用户的存款列表中删除指定存款ID
    bool removeDepositFromUserList(const std::string& username, const std::string& deposit_id);

public:
    DepositManager(AccountManager& am);

    // 连接到Redis
    bool connect();

    // 创建一个新存款
    bool createDeposit(const std::string& username, double amount, int deposit_type, int deposit_term = 0);

    // 计算存款利息
    double calculateInterest(const Deposit& deposit, int seconds);

    // 获取用户的所有存款
    std::vector<Deposit> getUserDeposits(const std::string& username);

    // 获取特定存款的详细信息
    Deposit getDepositDetails(const std::string& username, const std::string& deposit_id);

    // 从存款中取出资金（含利息）
    bool withdrawDeposit(const std::string& username, const std::string& deposit_id, double amount);

    // Redis键辅助函数
    static std::string getUserDepositCounterKey(const std::string& username);
    static std::string getUserDepositsKey(const std::string& username);
    static std::string getDepositKey(const std::string& username, const std::string& deposit_id);
};

#endif // DEPOSIT_MANAGER_H