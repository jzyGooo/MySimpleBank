// DepositManager.cpp - Implementation of deposit management functions with Redis
#include "DepositManager.h"
#include "Serializer.h"
#include <iostream>
#include <algorithm>

// Redis key prefixes
const std::string USER_DEPOSIT_COUNTER_KEY_PREFIX = "user:deposit_counter:";
const std::string USER_DEPOSITS_KEY_PREFIX = "user:deposits:";
const std::string DEPOSIT_KEY_PREFIX = "deposit:";

DepositManager::DepositManager(AccountManager& am) 
    : accountManager(am), redis("localhost", 6379) {
}

bool DepositManager::connect() {
    return redis.connect();
}

std::string DepositManager::getUserDepositCounterKey(const std::string& username) {
    return USER_DEPOSIT_COUNTER_KEY_PREFIX + username;
}

std::string DepositManager::getUserDepositsKey(const std::string& username) {
    return USER_DEPOSITS_KEY_PREFIX + username;
}

std::string DepositManager::getDepositKey(const std::string& username, const std::string& deposit_id) {
    return DEPOSIT_KEY_PREFIX + username + ":" + deposit_id;
}

std::string DepositManager::generateNextDepositId(const std::string& username) {
    std::lock_guard<std::mutex> lock(counterMutex);
    
    // 获取用户的存款计数器
    std::string counterKey = getUserDepositCounterKey(username);
    std::string counterStr = redis.get(counterKey);
    
    int counter = 1;
    if (!counterStr.empty()) {
        counter = std::stoi(counterStr) + 1;
    }
    
    // 更新计数器
    redis.set(counterKey, std::to_string(counter));
    
    // 生成格式为 "username-sequence" 的存款ID
    return username + "-" + std::to_string(counter);
}

bool DepositManager::createDeposit(const std::string& username, double amount, int deposit_type, int deposit_term) {
    User* user = accountManager.getUser(username);
    if (!user) {
        return false;
    }

    // 验证参数
    if (deposit_type == DEMAND_DEPOSIT) {
        if (amount <= 0) {
            delete user;
            return false;
        }
    }
    else if (deposit_type == TIME_DEPOSIT) {
        // 定期存款最低金额为10000
        if (amount <= 10000 || (deposit_term != TWO_MINUTES && deposit_term != THREE_MINUTES && deposit_term != FIVE_MINUTES)) {
            delete user;
            return false;
        }
    }
    else {
        delete user;
        return false;
    }

    // 检查余额是否充足
    if (user->balance < amount) {
        delete user;
        return false;
    }

    // 从余额中扣除金额
    user->balance -= amount;

    // 更新用户余额
    bool userUpdated = accountManager.updateUser(*user);
    if (!userUpdated) {
        delete user;
        return false;
    }

    // 生成唯一的存款ID
    std::string depositId = generateNextDepositId(username);

    // 创建新存款
    Deposit new_deposit;
    new_deposit.id = depositId;
    new_deposit.username = username;
    new_deposit.amount = amount;
    new_deposit.type = static_cast<DepositType>(deposit_type);
    new_deposit.term = static_cast<TimeDepositTerm>(deposit_term);
    new_deposit.depositTime = time(nullptr);
    new_deposit.isMatured = false;

    // 序列化存款信息
    std::string serialized = Serializer::serializeDeposit(new_deposit);
    
    // 存储存款信息到Redis
    bool depositStored = redis.set(getDepositKey(username, depositId), serialized);
    
    // 将存款ID添加到用户的存款列表
    bool idAdded = redis.rpush(getUserDepositsKey(username), depositId);
    
    delete user;
    return depositStored && idAdded;
}

double DepositManager::calculateInterest(const Deposit& deposit, int seconds) {
    double interest_rate = 0.0;
    double total_interest = 0.0;

    if (deposit.type == DEMAND_DEPOSIT) {
        // 活期存款利率：每秒0.03%
        interest_rate = 0.0003;
        total_interest = deposit.amount * interest_rate * seconds;
    }
    else if (deposit.type == TIME_DEPOSIT) {
        // 计算定期存款利息
        int minutes = seconds / 60;

        switch (deposit.term) {
        case TWO_MINUTES:
            // 2分钟期限：每分钟0.07%
            interest_rate = 0.0007;
            break;
        case THREE_MINUTES:
            // 3分钟期限：每分钟0.09%
            interest_rate = 0.0009;
            break;
        case FIVE_MINUTES:
            // 5分钟期限：每分钟0.1%
            interest_rate = 0.001;
            break;
        default:
            return 0.0;
        }

        total_interest = deposit.amount * interest_rate * minutes;
    }

    return total_interest;
}

std::vector<Deposit> DepositManager::getUserDeposits(const std::string& username) {
    std::vector<Deposit> deposits;
    
    // 获取用户的所有存款ID
    std::vector<std::string> depositIds = redis.lrange(getUserDepositsKey(username), 0, -1);
    
    // 获取每个存款的详细信息
    for (const auto& depositId : depositIds) {
        std::string serialized = redis.get(getDepositKey(username, depositId));
        if (!serialized.empty()) {
            Deposit deposit = Serializer::deserializeDeposit(serialized);
            deposits.push_back(deposit);
        }
    }
    
    return deposits;
}

Deposit DepositManager::getDepositDetails(const std::string& username, const std::string& deposit_id) {
    // 从Redis获取存款信息
    std::string serialized = redis.get(getDepositKey(username, deposit_id));
    
    if (!serialized.empty()) {
        return Serializer::deserializeDeposit(serialized);
    }
    
    // 找不到存款，返回空对象
    std::cerr << "找不到指定ID的存款: " << deposit_id << std::endl;
    return Deposit();
}

// 从用户的存款列表中删除指定存款ID
bool DepositManager::removeDepositFromUserList(const std::string& username, const std::string& deposit_id) {
    // 获取用户所有存款ID
    std::vector<std::string> depositIds = redis.lrange(getUserDepositsKey(username), 0, -1);
    
    // 从Redis中删除整个列表
    redis.del(getUserDepositsKey(username));
    
    // 创建一个新列表，不包含要删除的ID
    bool found = false;
    for (const auto& id : depositIds) {
        if (id != deposit_id) {
            redis.rpush(getUserDepositsKey(username), id);
        } else {
            found = true;
        }
    }
    
    return found;
}

bool DepositManager::withdrawDeposit(const std::string& username, const std::string& deposit_id, double amount) {
    User* user = accountManager.getUser(username);
    if (!user) {
        return false;
    }

    // 获取存款详情
    std::string serialized = redis.get(getDepositKey(username, deposit_id));
    if (serialized.empty()) {
        delete user;
        return false; // 未找到指定存款
    }
    
    Deposit deposit = Serializer::deserializeDeposit(serialized);

    // 验证取款金额
    if (amount <= 0 || amount > deposit.amount) {
        delete user;
        return false;
    }

    // 检查是否允许取款
    time_t current_time = time(nullptr);
    time_t elapsed_seconds = current_time - deposit.depositTime;
    double actual_amount = 0.0; // 实际取出金额（含利息）

    if (deposit.type == DEMAND_DEPOSIT) {
        // 活期存款：利息 = 本金 * 利率 * 秒数
        double interest_rate = 0.0003; // 0.03%

        // 计算按比例的利息
        double proportion = amount / deposit.amount;
        double interest = deposit.amount * interest_rate * elapsed_seconds * proportion;
        actual_amount = amount + interest;

        // 更新用户余额
        user->balance += actual_amount;

        // 更新存款金额
        if (amount >= deposit.amount) {
            // 如果取出全部金额，删除该存款
            redis.del(getDepositKey(username, deposit_id));
            
            // 从用户的存款列表中移除此存款ID
            removeDepositFromUserList(username, deposit_id);
        }
        else {
            // 否则只减少存款金额
            deposit.amount -= amount;
            std::string updatedSerialized = Serializer::serializeDeposit(deposit);
            redis.set(getDepositKey(username, deposit_id), updatedSerialized);
        }

        // 更新用户信息
        bool updated = accountManager.updateUser(*user);
        delete user;
        return updated;
    }
    else if (deposit.type == TIME_DEPOSIT) {
        // 定期存款：检查是否到期
        int term_seconds = deposit.term * 60; // 转换分钟为秒

        if (elapsed_seconds < term_seconds) {
            // 未到期，不允许取款
            delete user;
            return false;
        }

        // 已到期，计算利息
        double interest_rate = 0.0;
        switch (deposit.term) {
        case TWO_MINUTES:
            interest_rate = 0.0007; // 0.07%
            break;
        case THREE_MINUTES:
            interest_rate = 0.0009; // 0.09%
            break;
        case FIVE_MINUTES:
            interest_rate = 0.001; // 0.1%
            break;
        default:
            delete user;
            return false;
        }

        // 计算已过的完整分钟数
        int elapsed_minutes = elapsed_seconds / 60;

        // 计算按比例的利息
        double proportion = amount / deposit.amount;
        double interest = deposit.amount * interest_rate * elapsed_minutes * proportion;
        actual_amount = amount + interest;

        // 更新用户余额
        user->balance += actual_amount;

        // 更新存款金额
        if (amount >= deposit.amount) {
            // 如果取出全部金额，删除该存款
            redis.del(getDepositKey(username, deposit_id));
            
            // 从用户的存款列表中移除此存款ID
            removeDepositFromUserList(username, deposit_id);
        }
        else {
            // 否则只减少存款金额
            deposit.amount -= amount;
            std::string updatedSerialized = Serializer::serializeDeposit(deposit);
            redis.set(getDepositKey(username, deposit_id), updatedSerialized);
        }

        // 更新用户信息
        bool updated = accountManager.updateUser(*user);
        delete user;
        return updated;
    }

    delete user;
    return false; // 未找到指定存款或存款类型错误
}