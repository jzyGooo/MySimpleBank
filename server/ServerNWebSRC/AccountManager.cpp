// AccountManager.cpp - Implementation of account management with Redis
#include "AccountManager.h"
#include "Serializer.h"
#include <iostream>

// Redis key prefixes
const std::string USER_KEY_PREFIX = "user:";
const std::string USERS_LIST_KEY = "users";

AccountManager::AccountManager(const std::string& redis_host, int redis_port, const std::string& redis_password) 
    : redis(redis_host, redis_port, redis_password) {
}

AccountManager::~AccountManager() {
}

bool AccountManager::connect() {
    return redis.connect();
}

std::string AccountManager::getUserKey(const std::string& username) {
    return USER_KEY_PREFIX + username;
}

std::string AccountManager::getUsersListKey() {
    return USERS_LIST_KEY;
}

bool AccountManager::registerUser(const std::string& username, const std::string& password, int account_type) {
    std::lock_guard<std::mutex> lock(users_mutex);

    // Check if username already exists
    if (redis.exists(getUserKey(username))) {
        return false;
    }

    // Create new user
    User new_user;
    new_user.username = username;
    new_user.password = password;
    new_user.type = static_cast<AccountType>(account_type);
    new_user.balance = 0.0;

    // 序列化并存储用户
    std::string serialized = Serializer::serializeUser(new_user);
    bool success = redis.set(getUserKey(username), serialized);
    
    // 将用户添加到用户列表
    if (success) {
        redis.rpush(getUsersListKey(), username);
    }
    
    return success;
}

bool AccountManager::authenticateUser(const std::string& username, const std::string& password) {
    std::lock_guard<std::mutex> lock(users_mutex);

    // 检查用户是否存在
    if (!redis.exists(getUserKey(username))) {
        return false;
    }

    // 获取用户信息并验证密码
    std::string serialized = redis.get(getUserKey(username));
    if (serialized.empty()) {
        return false;
    }
    
    User user = Serializer::deserializeUser(serialized);
    return user.password == password;
}

User* AccountManager::getUser(const std::string& username) {
    std::lock_guard<std::mutex> lock(users_mutex);

    // 检查用户是否存在
    if (!redis.exists(getUserKey(username))) {
        return nullptr;
    }

    // 获取用户信息
    std::string serialized = redis.get(getUserKey(username));
    if (serialized.empty()) {
        return nullptr;
    }
    
    // 反序列化为用户对象
    User* user = new User(Serializer::deserializeUser(serialized));
    return user;
}

bool AccountManager::updateUser(const User& user) {
    std::lock_guard<std::mutex> lock(users_mutex);
    
    // 序列化并存储用户
    std::string serialized = Serializer::serializeUser(user);
    return redis.set(getUserKey(user.username), serialized);
}

std::map<std::string, User> AccountManager::getAllUsers() {
    std::lock_guard<std::mutex> lock(users_mutex);
    std::map<std::string, User> users;
    
    // 获取所有用户名
    std::vector<std::string> usernames = redis.lrange(getUsersListKey(), 0, -1);
    
    // 获取每个用户的详细信息
    for (const auto& username : usernames) {
        std::string serialized = redis.get(getUserKey(username));
        if (!serialized.empty()) {
            User user = Serializer::deserializeUser(serialized);
            users[username] = user;
        }
    }
    
    return users;
}