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
    std::mutex counterMutex; // �������������ʵĻ�����

    // Ϊָ���û�������һ�����ID
    std::string generateNextDepositId(const std::string& username);

    // ���û��Ĵ���б���ɾ��ָ�����ID
    bool removeDepositFromUserList(const std::string& username, const std::string& deposit_id);

public:
    DepositManager(AccountManager& am);

    // ���ӵ�Redis
    bool connect();

    // ����һ���´��
    bool createDeposit(const std::string& username, double amount, int deposit_type, int deposit_term = 0);

    // ��������Ϣ
    double calculateInterest(const Deposit& deposit, int seconds);

    // ��ȡ�û������д��
    std::vector<Deposit> getUserDeposits(const std::string& username);

    // ��ȡ�ض�������ϸ��Ϣ
    Deposit getDepositDetails(const std::string& username, const std::string& deposit_id);

    // �Ӵ����ȡ���ʽ𣨺���Ϣ��
    bool withdrawDeposit(const std::string& username, const std::string& deposit_id, double amount);

    // Redis����������
    static std::string getUserDepositCounterKey(const std::string& username);
    static std::string getUserDepositsKey(const std::string& username);
    static std::string getDepositKey(const std::string& username, const std::string& deposit_id);
};

#endif // DEPOSIT_MANAGER_H