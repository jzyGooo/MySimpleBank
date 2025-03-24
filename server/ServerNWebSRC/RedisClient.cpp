#include "RedisClient.h"
#include <iostream>

RedisClient::RedisClient(const std::string& host, int port, const std::string& password)
    : context(nullptr), host(host), port(port), password(password) {
}

RedisClient::~RedisClient() {
    disconnect();
}

bool RedisClient::connect() {
    // 断开已有连接
    disconnect();
    
    // 连接Redis服务器
    context = redisConnect(host.c_str(), port);
    
    if (context == nullptr || context->err) {
        if (context) {
            std::cerr << "Redis连接错误: " << context->errstr << std::endl;
            disconnect();
        } else {
            std::cerr << "Redis连接错误: 无法分配Redis上下文" << std::endl;
        }
        return false;
    }
    
    // 如果有密码，进行认证
    if (!password.empty()) {
        redisReply* reply = (redisReply*)redisCommand(context, "AUTH %s", password.c_str());
        if (reply == nullptr) {
            std::cerr << "Redis认证错误: 无法获取回复" << std::endl;
            disconnect();
            return false;
        }
        
        bool auth_success = (reply->type != REDIS_REPLY_ERROR);
        freeReply(reply);
        
        if (!auth_success) {
            std::cerr << "Redis认证失败" << std::endl;
            disconnect();
            return false;
        }
    }
    
    std::cout << "Redis连接成功" << std::endl;
    return true;
}

void RedisClient::disconnect() {
    if (context != nullptr) {
        redisFree(context);
        context = nullptr;
    }
}

bool RedisClient::isConnected() const {
    return context != nullptr && !context->err;
}

void RedisClient::freeReply(redisReply* reply) {
    if (reply != nullptr) {
        freeReplyObject(reply);
    }
}

bool RedisClient::set(const std::string& key, const std::string& value) {
    if (!isConnected()) {
        std::cerr << "Redis未连接" << std::endl;
        return false;
    }
    
    redisReply* reply = (redisReply*)redisCommand(context, "SET %s %s", key.c_str(), value.c_str());
    if (reply == nullptr) {
        std::cerr << "Redis SET命令错误: 无法获取回复" << std::endl;
        return false;
    }
    
    bool success = (reply->type != REDIS_REPLY_ERROR);
    freeReply(reply);
    
    return success;
}

std::string RedisClient::get(const std::string& key) {
    if (!isConnected()) {
        std::cerr << "Redis未连接" << std::endl;
        return "";
    }
    
    redisReply* reply = (redisReply*)redisCommand(context, "GET %s", key.c_str());
    if (reply == nullptr) {
        std::cerr << "Redis GET命令错误: 无法获取回复" << std::endl;
        return "";
    }
    
    std::string value;
    if (reply->type == REDIS_REPLY_STRING) {
        value = reply->str;
    }
    
    freeReply(reply);
    return value;
}

bool RedisClient::exists(const std::string& key) {
    if (!isConnected()) {
        std::cerr << "Redis未连接" << std::endl;
        return false;
    }
    
    redisReply* reply = (redisReply*)redisCommand(context, "EXISTS %s", key.c_str());
    if (reply == nullptr) {
        std::cerr << "Redis EXISTS命令错误: 无法获取回复" << std::endl;
        return false;
    }
    
    bool exists = (reply->integer == 1);
    freeReply(reply);
    
    return exists;
}

bool RedisClient::del(const std::string& key) {
    if (!isConnected()) {
        std::cerr << "Redis未连接" << std::endl;
        return false;
    }
    
    redisReply* reply = (redisReply*)redisCommand(context, "DEL %s", key.c_str());
    if (reply == nullptr) {
        std::cerr << "Redis DEL命令错误: 无法获取回复" << std::endl;
        return false;
    }
    
    bool success = (reply->integer == 1);
    freeReply(reply);
    
    return success;
}

bool RedisClient::hset(const std::string& key, const std::string& field, const std::string& value) {
    if (!isConnected()) {
        std::cerr << "Redis未连接" << std::endl;
        return false;
    }
    
    redisReply* reply = (redisReply*)redisCommand(context, "HSET %s %s %s", 
                                              key.c_str(), field.c_str(), value.c_str());
    if (reply == nullptr) {
        std::cerr << "Redis HSET命令错误: 无法获取回复" << std::endl;
        return false;
    }
    
    bool success = (reply->type != REDIS_REPLY_ERROR);
    freeReply(reply);
    
    return success;
}

std::string RedisClient::hget(const std::string& key, const std::string& field) {
    if (!isConnected()) {
        std::cerr << "Redis未连接" << std::endl;
        return "";
    }
    
    redisReply* reply = (redisReply*)redisCommand(context, "HGET %s %s", key.c_str(), field.c_str());
    if (reply == nullptr) {
        std::cerr << "Redis HGET命令错误: 无法获取回复" << std::endl;
        return "";
    }
    
    std::string value;
    if (reply->type == REDIS_REPLY_STRING) {
        value = reply->str;
    }
    
    freeReply(reply);
    return value;
}

bool RedisClient::hexists(const std::string& key, const std::string& field) {
    if (!isConnected()) {
        std::cerr << "Redis未连接" << std::endl;
        return false;
    }
    
    redisReply* reply = (redisReply*)redisCommand(context, "HEXISTS %s %s", key.c_str(), field.c_str());
    if (reply == nullptr) {
        std::cerr << "Redis HEXISTS命令错误: 无法获取回复" << std::endl;
        return false;
    }
    
    bool exists = (reply->integer == 1);
    freeReply(reply);
    
    return exists;
}

bool RedisClient::hdel(const std::string& key, const std::string& field) {
    if (!isConnected()) {
        std::cerr << "Redis未连接" << std::endl;
        return false;
    }
    
    redisReply* reply = (redisReply*)redisCommand(context, "HDEL %s %s", key.c_str(), field.c_str());
    if (reply == nullptr) {
        std::cerr << "Redis HDEL命令错误: 无法获取回复" << std::endl;
        return false;
    }
    
    bool success = (reply->integer == 1);
    freeReply(reply);
    
    return success;
}

std::map<std::string, std::string> RedisClient::hgetall(const std::string& key) {
    std::map<std::string, std::string> result;
    
    if (!isConnected()) {
        std::cerr << "Redis未连接" << std::endl;
        return result;
    }
    
    redisReply* reply = (redisReply*)redisCommand(context, "HGETALL %s", key.c_str());
    if (reply == nullptr) {
        std::cerr << "Redis HGETALL命令错误: 无法获取回复" << std::endl;
        return result;
    }
    
    if (reply->type == REDIS_REPLY_ARRAY) {
        for (size_t i = 0; i < reply->elements; i += 2) {
            if (i + 1 < reply->elements) {
                std::string field = reply->element[i]->str;
                std::string value = reply->element[i + 1]->str;
                result[field] = value;
            }
        }
    }
    
    freeReply(reply);
    return result;
}

bool RedisClient::lpush(const std::string& key, const std::string& value) {
    if (!isConnected()) {
        std::cerr << "Redis未连接" << std::endl;
        return false;
    }
    
    redisReply* reply = (redisReply*)redisCommand(context, "LPUSH %s %s", key.c_str(), value.c_str());
    if (reply == nullptr) {
        std::cerr << "Redis LPUSH命令错误: 无法获取回复" << std::endl;
        return false;
    }
    
    bool success = (reply->type != REDIS_REPLY_ERROR);
    freeReply(reply);
    
    return success;
}

bool RedisClient::rpush(const std::string& key, const std::string& value) {
    if (!isConnected()) {
        std::cerr << "Redis未连接" << std::endl;
        return false;
    }
    
    redisReply* reply = (redisReply*)redisCommand(context, "RPUSH %s %s", key.c_str(), value.c_str());
    if (reply == nullptr) {
        std::cerr << "Redis RPUSH命令错误: 无法获取回复" << std::endl;
        return false;
    }
    
    bool success = (reply->type != REDIS_REPLY_ERROR);
    freeReply(reply);
    
    return success;
}

std::vector<std::string> RedisClient::lrange(const std::string& key, int start, int stop) {
    std::vector<std::string> result;
    
    if (!isConnected()) {
        std::cerr << "Redis未连接" << std::endl;
        return result;
    }
    
    redisReply* reply = (redisReply*)redisCommand(context, "LRANGE %s %d %d", key.c_str(), start, stop);
    if (reply == nullptr) {
        std::cerr << "Redis LRANGE命令错误: 无法获取回复" << std::endl;
        return result;
    }
    
    if (reply->type == REDIS_REPLY_ARRAY) {
        for (size_t i = 0; i < reply->elements; i++) {
            result.push_back(reply->element[i]->str);
        }
    }
    
    freeReply(reply);
    return result;
}

bool RedisClient::multi() {
    if (!isConnected()) {
        std::cerr << "Redis未连接" << std::endl;
        return false;
    }
    
    redisReply* reply = (redisReply*)redisCommand(context, "MULTI");
    if (reply == nullptr) {
        std::cerr << "Redis MULTI命令错误: 无法获取回复" << std::endl;
        return false;
    }
    
    bool success = (reply->type != REDIS_REPLY_ERROR);
    freeReply(reply);
    
    return success;
}

bool RedisClient::exec() {
    if (!isConnected()) {
        std::cerr << "Redis未连接" << std::endl;
        return false;
    }
    
    redisReply* reply = (redisReply*)redisCommand(context, "EXEC");
    if (reply == nullptr) {
        std::cerr << "Redis EXEC命令错误: 无法获取回复" << std::endl;
        return false;
    }
    
    bool success = (reply->type != REDIS_REPLY_ERROR);
    freeReply(reply);
    
    return success;
}

bool RedisClient::discard() {
    if (!isConnected()) {
        std::cerr << "Redis未连接" << std::endl;
        return false;
    }
    
    redisReply* reply = (redisReply*)redisCommand(context, "DISCARD");
    if (reply == nullptr) {
        std::cerr << "Redis DISCARD命令错误: 无法获取回复" << std::endl;
        return false;
    }
    
    bool success = (reply->type != REDIS_REPLY_ERROR);
    freeReply(reply);
    
    return success;
}

bool RedisClient::expire(const std::string& key, int seconds) {
    if (!isConnected()) {
        std::cerr << "Redis未连接" << std::endl;
        return false;
    }
    
    redisReply* reply = (redisReply*)redisCommand(context, "EXPIRE %s %d", key.c_str(), seconds);
    if (reply == nullptr) {
        std::cerr << "Redis EXPIRE命令错误: 无法获取回复" << std::endl;
        return false;
    }
    
    bool success = (reply->integer == 1);
    freeReply(reply);
    
    return success;
}

bool RedisClient::flushdb() {
    if (!isConnected()) {
        std::cerr << "Redis未连接" << std::endl;
        return false;
    }
    
    redisReply* reply = (redisReply*)redisCommand(context, "FLUSHDB");
    if (reply == nullptr) {
        std::cerr << "Redis FLUSHDB命令错误: 无法获取回复" << std::endl;
        return false;
    }
    
    bool success = (reply->type != REDIS_REPLY_ERROR);
    freeReply(reply);
    
    return success;
}

std::string RedisClient::getLastError() const {
    if (context && context->err) {
        return context->errstr;
    }
    return "No error";
}