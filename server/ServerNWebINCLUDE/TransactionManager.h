// TransactionManager.h - Handles deposits, withdrawals, and transfers with Redis
#ifndef TRANSACTION_MANAGER_H
#define TRANSACTION_MANAGER_H

#include <string>
#include <vector>
#include <mutex>
#include "AccountManager.h"
#include "RedisClient.h"

class TransactionManager {
private:
    AccountManager& accountManager;
    RedisClient redis;
    std::mutex transaction_mutex;
    int transaction_counter = 0;

    // 生成唯一交易ID
    std::string generateTransactionId();

    // 创建并保存交易记录
    void recordTransaction(const std::string& username, TransactionType type,
        double amount, double balance_after,
        const std::string& counterparty = "",
        const std::string& description = "");

public:
    TransactionManager(AccountManager& am);

    // 连接到Redis
    bool connect();

    // 存款
    bool deposit(const std::string& username, double amount);

    // 取款
    bool withdraw(const std::string& username, double amount);

    // 转账
    bool transfer(const std::string& from_username, const std::string& to_username, double amount);

    // 获取余额
    double getBalance(const std::string& username);

    // 获取用户交易历史
    std::vector<TransactionRecord> getTransactionHistory(const std::string& username);

    // Redis键辅助函数
    static std::string getTransactionCounterKey();
    static std::string getUserTransactionsKey(const std::string& username);
};

#endif // TRANSACTION_MANAGER_H