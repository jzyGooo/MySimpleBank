#ifndef REDIS_CLIENT_H
#define REDIS_CLIENT_H

#include <string>
#include <map>
#include <vector>
#include <hiredis/hiredis.h>
#include "Common.h"

class RedisClient {
private:
    redisContext* context;
    std::string host;
    int port;
    std::string password;

    // 释放Redis回复对象
    void freeReply(redisReply* reply);

public:
    RedisClient(const std::string& host = "localhost", int port = 6379, const std::string& password = "");
    ~RedisClient();

    // 连接到Redis服务器
    bool connect();

    // 断开连接
    void disconnect();

    // 检查连接状态
    bool isConnected() const;

    // 基本操作
    bool set(const std::string& key, const std::string& value);
    std::string get(const std::string& key);
    bool exists(const std::string& key);
    bool del(const std::string& key);

    // 哈希表操作
    bool hset(const std::string& key, const std::string& field, const std::string& value);
    std::string hget(const std::string& key, const std::string& field);
    bool hexists(const std::string& key, const std::string& field);
    bool hdel(const std::string& key, const std::string& field);
    std::map<std::string, std::string> hgetall(const std::string& key);

    // 列表操作
    bool lpush(const std::string& key, const std::string& value);
    bool rpush(const std::string& key, const std::string& value);
    std::vector<std::string> lrange(const std::string& key, int start, int stop);

    // 事务操作
    bool multi();
    bool exec();
    bool discard();

    // 键过期设置
    bool expire(const std::string& key, int seconds);

    // 清除数据库
    bool flushdb();

    // 错误处理
    std::string getLastError() const;
};

#endif // REDIS_CLIENT_H