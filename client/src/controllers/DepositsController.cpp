#include "controllers/DepositsController.h"
#include "InputHandler.h"
#include "ConsoleView.h"
#include <iostream>
#include <iomanip>

DepositsController::DepositsController(ApiClient& apiClient, SessionManager& sessionManager)
    : apiClient(apiClient), sessionManager(sessionManager) {
}

bool DepositsController::createDeposit() {
    std::cout << "\n===== Create New Deposit =====\n";
    
    if (!sessionManager.isLoggedIn()) {
        ConsoleView::showError("You must be logged in to create a deposit");
        return false;
    }
    
    int depositType;
    std::cout << "Select deposit type:\n";
    std::cout << "1. Demand Deposit (0.03% per second)\n";
    std::cout << "2. Time Deposit\n";
    std::cout << "Type (1-2): ";
    std::cin >> depositType;
    InputHandler::clearInputBuffer();
    
    if (depositType < 1 || depositType > 2) {
        ConsoleView::showError("Invalid deposit type");
        return false;
    }
    
    int depositTerm = 0;
    if (depositType == 2) {  // Time deposit
        std::cout << "Select deposit term:\n";
        std::cout << "2. 2 minutes (0.07% per minute)\n";
        std::cout << "3. 3 minutes (0.09% per minute)\n";
        std::cout << "5. 5 minutes (0.1% per minute)\n";
        std::cout << "Term: ";
        std::cin >> depositTerm;
        InputHandler::clearInputBuffer();
        
        if (depositTerm != 2 && depositTerm != 3 && depositTerm != 5) {
            ConsoleView::showError("Invalid deposit term");
            return false;
        }
    }
    
    double amount;
    std::cout << "Enter deposit amount (¥): ";
    std::cin >> amount;
    InputHandler::clearInputBuffer();
    
    if (amount <= 0) {
        ConsoleView::showError("Amount must be greater than zero");
        return false;
    }
    
    if (depositType == 2 && amount < 10000) {
        ConsoleView::showError("Time deposits require a minimum amount of ¥10,000");
        return false;
    }
    
    std::map<std::string, std::string> params = {
        {"username", sessionManager.getCurrentUsername()},
        {"amount", std::to_string(amount)},
        {"deposit_type", std::to_string(depositType)}
    };
    
    if (depositType == 2) {
        params["deposit_term"] = std::to_string(depositTerm);
    }
    
    ConsoleView::showMessage("Creating deposit...");
    
    json response;
    bool success = apiClient.post("/api/create-deposit", params, response);
    
    if (!success) {
        ConsoleView::showError("Failed to create deposit: " + apiClient.getLastError());
        return false;
    }
    
    if (response["status"] == "success") {
        double newBalance = response["balance"].get<double>();
        ConsoleView::showSuccess("Deposit created successfully! New balance: ¥" + 
                                std::to_string(newBalance));
        return true;
    } else {
        ConsoleView::showError("Failed to create deposit: " + response["message"].get<std::string>());
        return false;
    }
}

void DepositsController::listDeposits() {
    std::cout << "\n===== My Deposits =====\n";
    
    if (!sessionManager.isLoggedIn()) {
        ConsoleView::showError("You must be logged in to view deposits");
        return;
    }
    
    std::map<std::string, std::string> params = {
        {"username", sessionManager.getCurrentUsername()}
    };
    
    ConsoleView::showMessage("Loading deposits...");
    
    json response;
    bool success = apiClient.get("/api/get-deposits", params, response);
    
    if (!success) {
        ConsoleView::showError("Failed to load deposits: " + apiClient.getLastError());
        return;
    }
    
    if (response["status"] == "success") {
        auto deposits = response["deposits"];
        
        if (deposits.empty()) {
            ConsoleView::showMessage("You don't have any deposits yet.");
            return;
        }
        
        std::vector<json> depositsList;
        for (const auto& deposit : deposits) {
            depositsList.push_back(deposit);
        }
        
        ConsoleView::showDeposits(depositsList);
        
        std::cout << "\nEnter deposit ID to view details, or press Enter to go back: ";
        std::string depositId = InputHandler::readLine();
        
        if (!depositId.empty()) {
            getDepositDetails(depositId);
        }
    } else {
        ConsoleView::showError("Failed to load deposits: " + response["message"].get<std::string>());
    }
}

bool DepositsController::getDepositDetails(const std::string& depositId) {
    if (!sessionManager.isLoggedIn()) {
        ConsoleView::showError("You must be logged in to view deposit details");
        return false;
    }
    
    std::map<std::string, std::string> params = {
        {"username", sessionManager.getCurrentUsername()},
        {"deposit_id", depositId}
    };
    
    ConsoleView::showMessage("Loading deposit details...");
    
    json response;
    bool success = apiClient.get("/api/get-deposit-details", params, response);
    
    if (!success) {
        ConsoleView::showError("Failed to load deposit details: " + apiClient.getLastError());
        return false;
    }
    
    if (response["status"] == "success") {
        auto deposit = response["deposit"];
        
        ConsoleView::showDepositDetails(deposit);
        
        // Check if withdrawal is possible
        bool canWithdraw = false;
        
        if (deposit["type"] == 1) {  // Demand deposit
            canWithdraw = true;
        } else if (deposit["type"] == 2 && deposit["isMatured"]) {  // Matured time deposit
            canWithdraw = true;
        }
        
        if (canWithdraw) {
            if (InputHandler::confirm("Do you want to withdraw from this deposit?")) {
                double maxAmount = deposit["amount"].get<double>();
                
                std::cout << "Enter amount to withdraw (max ¥" << std::fixed 
                          << std::setprecision(2) << maxAmount << "): ";
                double amount;
                std::cin >> amount;
                InputHandler::clearInputBuffer();
                
                if (amount <= 0 || amount > maxAmount) {
                    ConsoleView::showError("Invalid amount");
                    return false;
                }
                
                return withdrawDeposit(depositId, amount);
            }
        } else {
            ConsoleView::showMessage("This time deposit has not matured yet and cannot be withdrawn.");
        }
        
        return true;
    } else {
        ConsoleView::showError("Failed to load deposit details: " + response["message"].get<std::string>());
        return false;
    }
}

bool DepositsController::withdrawDeposit(const std::string& depositId, double amount) {
    if (!sessionManager.isLoggedIn()) {
        ConsoleView::showError("You must be logged in to withdraw from a deposit");
        return false;
    }
    
    std::map<std::string, std::string> params = {
        {"username", sessionManager.getCurrentUsername()},
        {"deposit_id", depositId},
        {"amount", std::to_string(amount)}
    };
    
    ConsoleView::showMessage("Processing withdrawal...");
    
    json response;
    bool success = apiClient.post("/api/withdraw-deposit", params, response);
    
    if (!success) {
        ConsoleView::showError("Withdrawal failed: " + apiClient.getLastError());
        return false;
    }
    
    if (response["status"] == "success") {
        double newBalance = response["balance"].get<double>();
        ConsoleView::showSuccess("Withdrawal successful! New balance: ¥" + 
                                std::to_string(newBalance));
        return true;
    } else {
        ConsoleView::showError("Withdrawal failed: " + response["message"].get<std::string>());
        return false;
    }
}