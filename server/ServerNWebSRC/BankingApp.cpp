// BankingApp.cpp - Implementation of the main application class (frontend functionality removed)
#include "BankingApp.h"
#include <iostream>
#include <csignal>

BankingApp::BankingApp(int port,
                     const std::string& redisHost, int redisPort, const std::string& redisPassword)
    : port(port),
      redisHost(redisHost),
      redisPort(redisPort),
      redisPassword(redisPassword),
      accountManager(redisHost, redisPort, redisPassword),
      transactionManager(accountManager),
      depositManager(accountManager),
      httpServer(port, accountManager, transactionManager, depositManager) {
}

bool BankingApp::initRedis() {
    std::cout << "Connecting to Redis at " << redisHost << ":" << redisPort << "..." << std::endl;
    
    // 连接AccountManager的Redis
    if (!accountManager.connect()) {
        std::cerr << "Failed to connect AccountManager to Redis" << std::endl;
        return false;
    }
    
    // 连接TransactionManager的Redis
    if (!transactionManager.connect()) {
        std::cerr << "Failed to connect TransactionManager to Redis" << std::endl;
        return false;
    }
    
    // 连接DepositManager的Redis
    if (!depositManager.connect()) {
        std::cerr << "Failed to connect DepositManager to Redis" << std::endl;
        return false;
    }
    
    std::cout << "All Redis connections established successfully" << std::endl;
    return true;
}

void BankingApp::run() {
    // 初始化Redis连接
    if (!initRedis()) {
        std::cerr << "Failed to initialize Redis connections" << std::endl;
        return;
    }
    
    // 启动HTTP API服务器
    if (!httpServer.start()) {
        std::cerr << "Failed to start HTTP API server" << std::endl;
        return;
    }
}

void BankingApp::stop() {
    // 停止HTTP服务器
    httpServer.stop();
    std::cout << "Banking application stopped" << std::endl;
}