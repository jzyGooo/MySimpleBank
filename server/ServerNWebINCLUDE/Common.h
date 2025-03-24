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
    DEMAND_DEPOSIT = 1,  // ���ڴ��
    TIME_DEPOSIT = 2     // ���ڴ��
};

// Time deposit term enumeration
enum TimeDepositTerm {
    TWO_MINUTES = 2,    // 2����
    THREE_MINUTES = 3,  // 3����
    FIVE_MINUTES = 5    // 5����
};

// Transaction type enumeration
enum TransactionType {
    DEPOSIT = 1,       // ���
    WITHDRAWAL = 2,    // ȡ��
    TRANSFER_IN = 3,   // ת��
    TRANSFER_OUT = 4   // ת��
};

// Transaction record structure
struct TransactionRecord {
    std::string id;               // ����ID
    TransactionType type;         // ��������
    std::string username;         // �û���
    std::string counterparty;     // ���׶Է���ת��ʱ��
    double amount;                // ���׽��
    double balance_after;         // ���׺����
    std::string description;      // ��������
    time_t timestamp;             // ����ʱ���

    // Default constructor
    TransactionRecord() : type(DEPOSIT), amount(0.0), balance_after(0.0), timestamp(0) {}
};

// Deposit structure
struct Deposit {
    std::string id;         // ���ID����ʽ: "username-sequence"
    std::string username;   // �û���
    double amount;          // �����
    DepositType type;       // �������
    TimeDepositTerm term;   // ���ڴ�����ޣ����Զ��ڴ�
    time_t depositTime;     // ���ʱ��
    bool isMatured;         // ���ڴ���Ƿ��ѵ���

    // Default constructor
    Deposit() : amount(0.0), type(DEMAND_DEPOSIT), term(TWO_MINUTES), depositTime(0), isMatured(false) {}
};

// User structure
struct User {
    std::string username;
    std::string password;
    AccountType type;
    double balance;
    std::vector<Deposit> deposits;                 // �û�����б�
    std::vector<TransactionRecord> transactions;   // �û����׼�¼
};

#endif // COMMON_H