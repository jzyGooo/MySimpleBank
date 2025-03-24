#include "Serializer.h"
#include <vector>

// 序列化和反序列化用户对象
std::string Serializer::serializeUser(const User& user) {
    std::ostringstream ss;
    ss << user.username << "|"
       << user.password << "|"
       << static_cast<int>(user.type) << "|"
       << user.balance;
    
    return ss.str();
}

User Serializer::deserializeUser(const std::string& data) {
    User user;
    std::istringstream ss(data);
    std::string token;
    
    // 使用'|'作为分隔符获取数据
    std::getline(ss, user.username, '|');
    std::getline(ss, user.password, '|');
    
    std::string typeStr;
    std::getline(ss, typeStr, '|');
    user.type = static_cast<AccountType>(std::stoi(typeStr));
    
    std::string balanceStr;
    std::getline(ss, balanceStr, '|');
    user.balance = std::stod(balanceStr);
    
    return user;
}

// 序列化和反序列化交易记录
std::string Serializer::serializeTransaction(const TransactionRecord& tx) {
    std::ostringstream ss;
    ss << tx.id << "|"
       << static_cast<int>(tx.type) << "|"
       << tx.username << "|"
       << tx.counterparty << "|"
       << tx.amount << "|"
       << tx.balance_after << "|"
       << tx.description << "|"
       << tx.timestamp;
    
    return ss.str();
}

TransactionRecord Serializer::deserializeTransaction(const std::string& data) {
    TransactionRecord tx;
    std::istringstream ss(data);
    std::string token;
    
    std::getline(ss, tx.id, '|');
    
    std::string typeStr;
    std::getline(ss, typeStr, '|');
    tx.type = static_cast<TransactionType>(std::stoi(typeStr));
    
    std::getline(ss, tx.username, '|');
    std::getline(ss, tx.counterparty, '|');
    
    std::string amountStr;
    std::getline(ss, amountStr, '|');
    tx.amount = std::stod(amountStr);
    
    std::string balanceStr;
    std::getline(ss, balanceStr, '|');
    tx.balance_after = std::stod(balanceStr);
    
    std::getline(ss, tx.description, '|');
    
    std::string timestampStr;
    std::getline(ss, timestampStr, '|');
    tx.timestamp = std::stol(timestampStr);
    
    return tx;
}

// 序列化和反序列化存款记录
std::string Serializer::serializeDeposit(const Deposit& deposit) {
    std::ostringstream ss;
    ss << deposit.id << "|"
       << deposit.username << "|"
       << deposit.amount << "|"
       << static_cast<int>(deposit.type) << "|"
       << static_cast<int>(deposit.term) << "|"
       << deposit.depositTime << "|"
       << (deposit.isMatured ? "1" : "0");
    
    return ss.str();
}

Deposit Serializer::deserializeDeposit(const std::string& data) {
    Deposit deposit;
    std::istringstream ss(data);
    std::string token;
    
    std::getline(ss, deposit.id, '|');
    std::getline(ss, deposit.username, '|');
    
    std::string amountStr;
    std::getline(ss, amountStr, '|');
    deposit.amount = std::stod(amountStr);
    
    std::string typeStr;
    std::getline(ss, typeStr, '|');
    deposit.type = static_cast<DepositType>(std::stoi(typeStr));
    
    std::string termStr;
    std::getline(ss, termStr, '|');
    deposit.term = static_cast<TimeDepositTerm>(std::stoi(termStr));
    
    std::string timeStr;
    std::getline(ss, timeStr, '|');
    deposit.depositTime = std::stol(timeStr);
    
    std::string maturedStr;
    std::getline(ss, maturedStr, '|');
    deposit.isMatured = (maturedStr == "1");
    
    return deposit;
}