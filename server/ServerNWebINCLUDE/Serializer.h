#ifndef SERIALIZER_H
#define SERIALIZER_H

#include <string>
#include <sstream>
#include "Common.h"

class Serializer {
public:
    // 用户对象序列化
    static std::string serializeUser(const User& user);
    static User deserializeUser(const std::string& data);

    // 交易记录序列化
    static std::string serializeTransaction(const TransactionRecord& transaction);
    static TransactionRecord deserializeTransaction(const std::string& data);

    // 存款序列化
    static std::string serializeDeposit(const Deposit& deposit);
    static Deposit deserializeDeposit(const std::string& data);
};

#endif // SERIALIZER_H