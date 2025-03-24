#ifndef TRANSACTION_CONTROLLER_H
#define TRANSACTION_CONTROLLER_H

#include "ApiClient.h"
#include "SessionManager.h"
#include <vector>

class TransactionController {
private:
    ApiClient& apiClient;
    SessionManager& sessionManager;

public:
    TransactionController(ApiClient& apiClient, SessionManager& sessionManager);

    // Get balance
    double getBalance();

    // Make a deposit
    bool makeDeposit();

    // Make a withdrawal
    bool makeWithdrawal();

    // Make a transfer
    bool makeTransfer();

    // Show transaction history
    void showTransactionHistory();
};

#endif // TRANSACTION_CONTROLLER_H