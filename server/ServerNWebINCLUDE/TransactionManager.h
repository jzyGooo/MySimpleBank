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

    // ����Ψһ����ID
    std::string generateTransactionId();

    // ���������潻�׼�¼
    void recordTransaction(const std::string& username, TransactionType type,
        double amount, double balance_after,
        const std::string& counterparty = "",
        const std::string& description = "");

public:
    TransactionManager(AccountManager& am);

    // ���ӵ�Redis
    bool connect();

    // ���
    bool deposit(const std::string& username, double amount);

    // ȡ��
    bool withdraw(const std::string& username, double amount);

    // ת��
    bool transfer(const std::string& from_username, const std::string& to_username, double amount);

    // ��ȡ���
    double getBalance(const std::string& username);

    // ��ȡ�û�������ʷ
    std::vector<TransactionRecord> getTransactionHistory(const std::string& username);

    // Redis����������
    static std::string getTransactionCounterKey();
    static std::string getUserTransactionsKey(const std::string& username);
};

#endif // TRANSACTION_MANAGER_H