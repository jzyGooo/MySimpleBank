// TransactionManager.cpp - Implementation of transaction functions with Redis
#include "TransactionManager.h"
#include "Serializer.h"
#include <ctime>
#include <iomanip>
#include <sstream>

// Redis key prefixes
const std::string TRANSACTION_COUNTER_KEY = "transaction:counter";
const std::string USER_TRANSACTIONS_KEY_PREFIX = "user:transactions:";

TransactionManager::TransactionManager(AccountManager& am) 
    : accountManager(am), redis("localhost", 6379) {
    // 尝试从Redis获取当前交易计数器值
    std::string counter_str = redis.get(getTransactionCounterKey());
    if (!counter_str.empty()) {
        transaction_counter = std::stoi(counter_str);
    }
}

bool TransactionManager::connect() {
    return redis.connect();
}

std::string TransactionManager::getTransactionCounterKey() {
    return TRANSACTION_COUNTER_KEY;
}

std::string TransactionManager::getUserTransactionsKey(const std::string& username) {
    return USER_TRANSACTIONS_KEY_PREFIX + username;
}

std::string TransactionManager::generateTransactionId() {
    std::lock_guard<std::mutex> lock(transaction_mutex);
    transaction_counter++;
    
    // 更新Redis中的计数器
    redis.set(getTransactionCounterKey(), std::to_string(transaction_counter));
    
    // 生成带时间戳的交易ID：TX-年月日-计数器
    std::time_t now = std::time(nullptr);
    std::tm* now_tm = std::localtime(&now);
    
    std::ostringstream oss;
    oss << "TX-"
        << std::setw(4) << std::setfill('0') << (now_tm->tm_year + 1900)
        << std::setw(2) << std::setfill('0') << (now_tm->tm_mon + 1)
        << std::setw(2) << std::setfill('0') << now_tm->tm_mday
        << "-" << std::setw(6) << std::setfill('0') << transaction_counter;
    
    return oss.str();
}

void TransactionManager::recordTransaction(const std::string& username, TransactionType type, 
                                          double amount, double balance_after, 
                                          const std::string& counterparty, 
                                          const std::string& description) {
    User* user = accountManager.getUser(username);
    if (!user) {
        return;
    }
    
    TransactionRecord record;
    record.id = generateTransactionId();
    record.type = type;
    record.username = username;
    record.counterparty = counterparty;
    record.amount = amount;
    record.balance_after = balance_after;
    record.description = description;
    record.timestamp = std::time(nullptr);
    
    // 序列化交易记录
    std::string serialized = Serializer::serializeTransaction(record);
    
    // 存储到Redis
    redis.rpush(getUserTransactionsKey(username), serialized);
    
    // 释放从getUser获取的内存
    delete user;
}

bool TransactionManager::deposit(const std::string& username, double amount) {
    if (amount <= 0) {
        return false;
    }

    User* user = accountManager.getUser(username);
    if (!user) {
        return false;
    }

    user->balance += amount;
    
    // 更新用户信息到Redis
    bool updated = accountManager.updateUser(*user);
    
    // 记录存款交易
    if (updated) {
        recordTransaction(username, DEPOSIT, amount, user->balance, "", "存款");
    }
    
    // 释放从getUser获取的内存
    delete user;
    
    return updated;
}

bool TransactionManager::withdraw(const std::string& username, double amount) {
    if (amount <= 0) {
        return false;
    }

    User* user = accountManager.getUser(username);
    if (!user || user->balance < amount) {
        delete user;
        return false;
    }

    user->balance -= amount;
    
    // 更新用户信息到Redis
    bool updated = accountManager.updateUser(*user);
    
    // 记录取款交易
    if (updated) {
        recordTransaction(username, WITHDRAWAL, amount, user->balance, "", "取款");
    }
    
    // 释放从getUser获取的内存
    delete user;
    
    return updated;
}

bool TransactionManager::transfer(const std::string& from_username, const std::string& to_username, double amount) {
    if (amount <= 0) {
        return false;
    }

    User* from_user = accountManager.getUser(from_username);
    User* to_user = accountManager.getUser(to_username);

    if (!from_user || !to_user || from_user->balance < amount) {
        delete from_user;
        delete to_user;
        return false;
    }

    from_user->balance -= amount;
    to_user->balance += amount;
    
    // 更新两个用户的信息到Redis
    bool from_updated = accountManager.updateUser(*from_user);
    bool to_updated = accountManager.updateUser(*to_user);
    
    if (from_updated && to_updated) {
        // 记录转出交易
        recordTransaction(from_username, TRANSFER_OUT, amount, from_user->balance, 
                         to_username, "转账给 " + to_username);
        
        // 记录转入交易
        recordTransaction(to_username, TRANSFER_IN, amount, to_user->balance, 
                         from_username, "收到来自 " + from_username + " 的转账");
    }
    
    // 释放从getUser获取的内存
    delete from_user;
    delete to_user;
    
    return from_updated && to_updated;
}

double TransactionManager::getBalance(const std::string& username) {
    User* user = accountManager.getUser(username);
    if (!user) {
        return -1.0;  // User does not exist
    }

    double balance = user->balance;
    
    // 释放从getUser获取的内存
    delete user;
    
    return balance;
}

std::vector<TransactionRecord> TransactionManager::getTransactionHistory(const std::string& username) {
    std::vector<TransactionRecord> transactions;
    
    // 从Redis获取用户的所有交易记录
    std::vector<std::string> serialized_txs = redis.lrange(getUserTransactionsKey(username), 0, -1);
    
    for (const auto& serialized : serialized_txs) {
        TransactionRecord tx = Serializer::deserializeTransaction(serialized);
        transactions.push_back(tx);
    }
    
    return transactions;
}