// AccountManager.h - User account management with Redis storage
#ifndef ACCOUNT_MANAGER_H
#define ACCOUNT_MANAGER_H

#include <string>
#include <map>
#include <mutex>
#include "Common.h"
#include "RedisClient.h"

class AccountManager {
private:
    RedisClient redis;
    std::mutex users_mutex;

public:
    AccountManager(const std::string& redis_host = "localhost",
        int redis_port = 6379,
        const std::string& redis_password = "");
    ~AccountManager();

    // Connect to Redis
    bool connect();

    // Register a new user
    bool registerUser(const std::string& username, const std::string& password, int account_type);

    // Authenticate a user
    bool authenticateUser(const std::string& username, const std::string& password);

    // Get user pointer by username (cached temporarily)
    User* getUser(const std::string& username);

    // Update user in Redis
    bool updateUser(const User& user);

    // Get all users
    std::map<std::string, User> getAllUsers();

    // Redis key helpers
    static std::string getUserKey(const std::string& username);
    static std::string getUsersListKey();
};

#endif // ACCOUNT_MANAGER_H