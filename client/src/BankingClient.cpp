#include "BankingClient.h"
#include "ConsoleView.h"
#include "InputHandler.h"
#include <iostream>
#include <iomanip>

BankingClient::BankingClient(const std::string& serverUrl)
    : apiClient(serverUrl),
      sessionManager(SessionManager::getInstance()),
      authController(apiClient, sessionManager),
      transactionController(apiClient, sessionManager),
      depositsController(apiClient, sessionManager),
      running(true) {
}

void BankingClient::showAuthMenu() {
    std::vector<std::string> options = {
        "登录",
        "注册"
    };
    
    int choice = ConsoleView::showMenu("银行系统 - 认证", options);
    
    switch (choice) {
        case 1:
            authController.login();
            break;
        case 2:
            authController.registerUser();
            break;
        case 0:
            running = false;
            break;
        default:
            ConsoleView::showError("无效选项");
            break;
    }
}

void BankingClient::showMainMenu() {
    // 显示用户信息和余额
    double balance = transactionController.getBalance();
    std::cout << "\n===== 银行系统 - 主菜单 =====\n";
    std::cout << "用户: " << sessionManager.getCurrentUsername() << "\n";
    std::cout << "余额: ¥" << std::fixed << std::setprecision(2) << balance << "\n";
    
    std::vector<std::string> options = {
        "存款",
        "取款",
        "转账",
        "创建存款",
        "管理存款",
        "交易历史",
        "退出登录"
    };
    
    int choice = ConsoleView::showMenu("", options);
    
    switch (choice) {
        case 1:
            transactionController.makeDeposit();
            break;
        case 2:
            transactionController.makeWithdrawal();
            break;
        case 3:
            transactionController.makeTransfer();
            break;
        case 4:
            depositsController.createDeposit();
            break;
        case 5:
            depositsController.listDeposits();
            break;
        case 6:
            transactionController.showTransactionHistory();
            break;
        case 7:
            authController.logout();
            break;
        case 0:
            running = false;
            break;
        default:
            ConsoleView::showError("无效选项");
            break;
    }
}

void BankingClient::run() {
    std::cout << "+-------------------------------+\n";
    std::cout << "|   银行系统命令行客户端        |\n";
    std::cout << "+-------------------------------+\n";
    
    while (running) {
        if (!sessionManager.isLoggedIn()) {
            showAuthMenu();
        } else {
            showMainMenu();
        }
    }
    
    std::cout << "感谢使用银行系统。再见！\n";
}