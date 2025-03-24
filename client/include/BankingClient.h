#ifndef BANKING_CLIENT_H
#define BANKING_CLIENT_H

#include "ApiClient.h"
#include "SessionManager.h"
#include "controllers/AuthController.h"
#include "controllers/TransactionController.h"
#include "controllers/DepositsController.h"

class BankingClient {
private:
    ApiClient apiClient;
    SessionManager& sessionManager;
    AuthController authController;
    TransactionController transactionController;
    DepositsController depositsController;

    bool running;

    // Show login/register menu
    void showAuthMenu();

    // Show main menu for logged-in users
    void showMainMenu();

public:
    BankingClient(const std::string& serverUrl);

    // Run the client application
    void run();
};

#endif // BANKING_CLIENT_H