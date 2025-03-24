#ifndef SERIALIZER_H
#define SERIALIZER_H

#include <string>
#include <sstream>
#include "Common.h"

class Serializer {
public:
    // �û��������л�
    static std::string serializeUser(const User& user);
    static User deserializeUser(const std::string& data);

    // ���׼�¼���л�
    static std::string serializeTransaction(const TransactionRecord& transaction);
    static TransactionRecord deserializeTransaction(const std::string& data);

    // ������л�
    static std::string serializeDeposit(const Deposit& deposit);
    static Deposit deserializeDeposit(const std::string& data);
};

#endif // SERIALIZER_H