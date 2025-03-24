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

    // �ͷ�Redis�ظ�����
    void freeReply(redisReply* reply);

public:
    RedisClient(const std::string& host = "localhost", int port = 6379, const std::string& password = "");
    ~RedisClient();

    // ���ӵ�Redis������
    bool connect();

    // �Ͽ�����
    void disconnect();

    // �������״̬
    bool isConnected() const;

    // ��������
    bool set(const std::string& key, const std::string& value);
    std::string get(const std::string& key);
    bool exists(const std::string& key);
    bool del(const std::string& key);

    // ��ϣ�����
    bool hset(const std::string& key, const std::string& field, const std::string& value);
    std::string hget(const std::string& key, const std::string& field);
    bool hexists(const std::string& key, const std::string& field);
    bool hdel(const std::string& key, const std::string& field);
    std::map<std::string, std::string> hgetall(const std::string& key);

    // �б����
    bool lpush(const std::string& key, const std::string& value);
    bool rpush(const std::string& key, const std::string& value);
    std::vector<std::string> lrange(const std::string& key, int start, int stop);

    // �������
    bool multi();
    bool exec();
    bool discard();

    // ����������
    bool expire(const std::string& key, int seconds);

    // ������ݿ�
    bool flushdb();

    // ������
    std::string getLastError() const;
};

#endif // REDIS_CLIENT_H