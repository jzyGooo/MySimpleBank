#include "controllers/AuthController.h"
#include "InputHandler.h"
#include "ConsoleView.h"
#include <iostream>

AuthController::AuthController(ApiClient& apiClient, SessionManager& sessionManager)
    : apiClient(apiClient), sessionManager(sessionManager) {
}

bool AuthController::login() {
    std::cout << "\n===== 用户登录 =====\n";
    
    std::string username, password;
    
    std::cout << "用户名: ";
    username = InputHandler::readLine();
    
    if (username.empty()) {
        ConsoleView::showMessage("登录已取消");
        return false;
    }
    
    std::cout << "密码: ";
    password = InputHandler::readPassword();
    
    if (password.empty()) {
        ConsoleView::showMessage("登录已取消");
        return false;
    }
    
    std::map<std::string, std::string> params = {
        {"username", username},
        {"password", password}
    };
    
    ConsoleView::showMessage("正在登录...");
    
    json response;
    bool success = apiClient.post("/api/login", params, response);
    
    if (!success) {
        ConsoleView::showError("登录失败: " + apiClient.getLastError());
        return false;
    }
    
    if (response["status"] == "success") {
        ConsoleView::showSuccess("登录成功!");
        sessionManager.initSession(username);
        return true;
    } else {
        ConsoleView::showError("登录失败: " + response["message"].get<std::string>());
        return false;
    }
}

bool AuthController::registerUser() {
    std::cout << "\n===== 用户注册 =====\n";
    
    std::string username, password;
    int accountType;
    
    std::cout << "用户名: ";
    username = InputHandler::readLine();
    
    if (username.empty()) {
        ConsoleView::showMessage("注册已取消");
        return false;
    }
    
    std::cout << "密码: ";
    password = InputHandler::readPassword();
    
    if (password.empty()) {
        ConsoleView::showMessage("注册已取消");
        return false;
    }
    
    std::cout << "账户类型:\n";
    std::cout << "1. 个人账户\n";
    std::cout << "2. 公共账户\n";
    std::cout << "3. 联名账户\n";
    std::cout << "选择 (1-3): ";
    accountType = InputHandler::readInt();
    
    if (accountType < 1 || accountType > 3) {
        ConsoleView::showError("无效的账户类型");
        return false;
    }
    
    std::map<std::string, std::string> params = {
        {"username", username},
        {"password", password},
        {"account_type", std::to_string(accountType)}
    };
    
    ConsoleView::showMessage("正在注册用户...");
    
    json response;
    bool success = apiClient.post("/api/register", params, response);
    
    if (!success) {
        ConsoleView::showError("注册失败: " + apiClient.getLastError());
        return false;
    }
    
    if (response["status"] == "success") {
        ConsoleView::showSuccess("注册成功!");
        return true;
    } else {
        // 检查是否是用户名已存在的错误
        std::string errorMsg = response["message"].get<std::string>();
        if (errorMsg.find("Username already exists") != std::string::npos) {
            ConsoleView::showError("注册失败: 用户名已存在，请选择其他用户名");
        } else {
            ConsoleView::showError("注册失败: " + errorMsg);
        }
        return false;
    }
}

void AuthController::logout() {
    sessionManager.clearSession();
    ConsoleView::showMessage("您已退出登录。");
}