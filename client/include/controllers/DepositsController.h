#ifndef DEPOSITS_CONTROLLER_H
#define DEPOSITS_CONTROLLER_H

#include "ApiClient.h"
#include "SessionManager.h"
#include <string>
#include <vector>

class DepositsController {
private:
    ApiClient& apiClient;
    SessionManager& sessionManager;

public:
    DepositsController(ApiClient& apiClient, SessionManager& sessionManager);

    // Create a new deposit
    bool createDeposit();

    // Get list of user deposits
    void listDeposits();

    // Get deposit details
    bool getDepositDetails(const std::string& depositId);

    // Withdraw from a deposit
    bool withdrawDeposit(const std::string& depositId, double amount);
};

#endif // DEPOSITS_CONTROLLER_H