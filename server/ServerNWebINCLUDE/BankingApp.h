// BankingApp.h - Main application class with Redis support (frontend functionality removed)
#ifndef BANKING_APP_H
#define BANKING_APP_H

#include <string>
#include "AccountManager.h"
#include "TransactionManager.h"
#include "DepositManager.h"
#include "HttpServer.h"

class BankingApp {
private:
    // Components
    AccountManager accountManager;
    TransactionManager transactionManager;
    DepositManager depositManager;
    HttpServer httpServer;

    // Configuration
    int port;

    // Redis configuration
    std::string redisHost;
    int redisPort;
    std::string redisPassword;

public:
    BankingApp(int port,
        const std::string& redisHost = "localhost",
        int redisPort = 6379,
        const std::string& redisPassword = "");

    // Run the application
    void run();

    // Stop the application
    void stop();

    // Initialize Redis connections
    bool initRedis();
};

#endif // BANKING_APP_H