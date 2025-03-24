#include "controllers/TransactionController.h"
#include "InputHandler.h"
#include "ConsoleView.h"
#include <iostream>
#include <iomanip>

TransactionController::TransactionController(ApiClient& apiClient, SessionManager& sessionManager)
    : apiClient(apiClient), sessionManager(sessionManager) {
}

double TransactionController::getBalance() {
    if (!sessionManager.isLoggedIn()) {
        ConsoleView::showError("You must be logged in to view your balance");
        return -1;
    }
    
    std::map<std::string, std::string> params = {
        {"username", sessionManager.getCurrentUsername()}
    };
    
    json response;
    bool success = apiClient.get("/api/balance", params, response);
    
    if (!success || response["status"] != "success") {
        ConsoleView::showError("Failed to get balance: " + 
                              (success ? response["message"].get<std::string>() : apiClient.getLastError()));
        return -1;
    }
    
    return response["balance"].get<double>();
}

bool TransactionController::makeDeposit() {
    std::cout << "\n===== Make Deposit =====\n";
    
    if (!sessionManager.isLoggedIn()) {
        ConsoleView::showError("You must be logged in to make a deposit");
        return false;
    }
    
    double amount;
    std::cout << "Enter amount to deposit (¥): ";
    std::cin >> amount;
    InputHandler::clearInputBuffer();
    
    if (amount <= 0) {
        ConsoleView::showError("Amount must be greater than zero");
        return false;
    }
    
    std::map<std::string, std::string> params = {
        {"username", sessionManager.getCurrentUsername()},
        {"amount", std::to_string(amount)}
    };
    
    ConsoleView::showMessage("Processing deposit...");
    
    json response;
    bool success = apiClient.post("/api/deposit", params, response);
    
    if (!success) {
        ConsoleView::showError("Deposit failed: " + apiClient.getLastError());
        return false;
    }
    
    if (response["status"] == "success") {
        double newBalance = response["balance"].get<double>();
        ConsoleView::showSuccess("Deposit successful! New balance: ¥" + 
                                std::to_string(newBalance));
        return true;
    } else {
        ConsoleView::showError("Deposit failed: " + response["message"].get<std::string>());
        return false;
    }
}

bool TransactionController::makeWithdrawal() {
    std::cout << "\n===== Make Withdrawal =====\n";
    
    if (!sessionManager.isLoggedIn()) {
        ConsoleView::showError("You must be logged in to make a withdrawal");
        return false;
    }
    
    double amount;
    std::cout << "Enter amount to withdraw (¥): ";
    std::cin >> amount;
    InputHandler::clearInputBuffer();
    
    if (amount <= 0) {
        ConsoleView::showError("Amount must be greater than zero");
        return false;
    }
    
    std::map<std::string, std::string> params = {
        {"username", sessionManager.getCurrentUsername()},
        {"amount", std::to_string(amount)}
    };
    
    ConsoleView::showMessage("Processing withdrawal...");
    
    json response;
    bool success = apiClient.post("/api/withdraw", params, response);
    
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

bool TransactionController::makeTransfer() {
    std::cout << "\n===== Make Transfer =====\n";
    
    if (!sessionManager.isLoggedIn()) {
        ConsoleView::showError("You must be logged in to make a transfer");
        return false;
    }
    
    std::string recipientUsername;
    double amount;
    
    std::cout << "Recipient username: ";
    recipientUsername = InputHandler::readLine();
    
    if (recipientUsername.empty()) {
        ConsoleView::showError("Recipient username cannot be empty");
        return false;
    }
    
    std::cout << "Enter amount to transfer (¥): ";
    std::cin >> amount;
    InputHandler::clearInputBuffer();
    
    if (amount <= 0) {
        ConsoleView::showError("Amount must be greater than zero");
        return false;
    }
    
    std::map<std::string, std::string> params = {
        {"username", sessionManager.getCurrentUsername()},
        {"to_username", recipientUsername},
        {"amount", std::to_string(amount)}
    };
    
    ConsoleView::showMessage("Processing transfer...");
    
    json response;
    bool success = apiClient.post("/api/transfer", params, response);
    
    if (!success) {
        ConsoleView::showError("Transfer failed: " + apiClient.getLastError());
        return false;
    }
    
    if (response["status"] == "success") {
        double newBalance = response["balance"].get<double>();
        ConsoleView::showSuccess("Transfer successful! New balance: ¥" + 
                                std::to_string(newBalance));
        return true;
    } else {
        ConsoleView::showError("Transfer failed: " + response["message"].get<std::string>());
        return false;
    }
}

void TransactionController::showTransactionHistory() {
    std::cout << "\n===== Transaction History =====\n";
    
    if (!sessionManager.isLoggedIn()) {
        ConsoleView::showError("You must be logged in to view transaction history");
        return;
    }
    
    std::map<std::string, std::string> params = {
        {"username", sessionManager.getCurrentUsername()}
    };
    
    ConsoleView::showMessage("Loading transaction history...");
    
    json response;
    bool success = apiClient.get("/api/transaction-history", params, response);
    
    if (!success) {
        ConsoleView::showError("Failed to load transaction history: " + apiClient.getLastError());
        return;
    }
    
    if (response["status"] == "success") {
        auto transactions = response["transactions"];
        
        if (transactions.empty()) {
            ConsoleView::showMessage("No transactions found.");
            return;
        }
        
        std::vector<json> txList;
        for (const auto& tx : transactions) {
            txList.push_back(tx);
        }
        
        ConsoleView::showTransactions(txList);
    } else {
        ConsoleView::showError("Failed to load transaction history: " + response["message"].get<std::string>());
    }
}