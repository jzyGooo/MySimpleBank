// Common.h - Common definitions and structures
#ifndef COMMON_H
#define COMMON_H

#include <string>
#include <vector>
#include <ctime>

// User account type enumeration
enum AccountType {
    PRIVATE_ACCOUNT = 1,
    PUBLIC_ACCOUNT = 2,
    JOINT_SIGNATURE_ACCOUNT = 3
};

// Deposit type enumeration
enum DepositType {
    DEMAND_DEPOSIT = 1,  // 活期存款
    TIME_DEPOSIT = 2     // 定期存款
};

// Time deposit term enumeration
enum TimeDepositTerm {
    TWO_MINUTES = 2,    // 2分钟
    THREE_MINUTES = 3,  // 3分钟
    FIVE_MINUTES = 5    // 5分钟
};

// Transaction type enumeration
enum TransactionType {
    DEPOSIT = 1,       // 存款
    WITHDRAWAL = 2,    // 取款
    TRANSFER_IN = 3,   // 转入
    TRANSFER_OUT = 4   // 转出
};

// Transaction record structure
struct TransactionRecord {
    std::string id;               // 交易ID
    TransactionType type;         // 交易类型
    std::string username;         // 用户名
    std::string counterparty;     // 交易对方（转账时）
    double amount;                // 交易金额
    double balance_after;         // 交易后余额
    std::string description;      // 交易描述
    time_t timestamp;             // 交易时间戳

    // Default constructor
    TransactionRecord() : type(DEPOSIT), amount(0.0), balance_after(0.0), timestamp(0) {}
};

// Deposit structure
struct Deposit {
    std::string id;         // 存款ID，格式: "username-sequence"
    std::string username;   // 用户名
    double amount;          // 存款金额
    DepositType type;       // 存款类型
    TimeDepositTerm term;   // 定期存款期限（仅对定期存款）
    time_t depositTime;     // 存款时间
    bool isMatured;         // 定期存款是否已到期

    // Default constructor
    Deposit() : amount(0.0), type(DEMAND_DEPOSIT), term(TWO_MINUTES), depositTime(0), isMatured(false) {}
};

// User structure
struct User {
    std::string username;
    std::string password;
    AccountType type;
    double balance;
    std::vector<Deposit> deposits;                 // 用户存款列表
    std::vector<TransactionRecord> transactions;   // 用户交易记录
};

#endif // COMMON_H