#ifndef CONSOLE_VIEW_H
#define CONSOLE_VIEW_H

#include <string>
#include <vector>
#include <functional>
#include "json.hpp"

using json = nlohmann::json;

class ConsoleView {
public:
    // Display a message
    static void showMessage(const std::string& message);

    // Display an error message
    static void showError(const std::string& message);

    // Display a success message
    static void showSuccess(const std::string& message);

    // Display a menu and get user choice
    static int showMenu(const std::string& title, const std::vector<std::string>& options);

    // Format a timestamp as a readable date/time string
    static std::string formatTimestamp(time_t timestamp);

    // Display a table of transactions
    static void showTransactions(const std::vector<json>& transactions);

    // Display a table of deposits
    static void showDeposits(const std::vector<json>& deposits);

    // Display deposit details
    static void showDepositDetails(const json& deposit);

    // Display account balance
    static void showBalance(double balance);

    // Clear screen
    static void clearScreen();
};

#endif // CONSOLE_VIEW_H