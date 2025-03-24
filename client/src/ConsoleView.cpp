#include "ConsoleView.h"
#include <iostream>
#include <iomanip>
#include <ctime>

void ConsoleView::showMessage(const std::string& message) {
    std::cout << message << std::endl;
}

void ConsoleView::showError(const std::string& message) {
    std::cout << "\033[1;31m错误: " << message << "\033[0m" << std::endl;
}

void ConsoleView::showSuccess(const std::string& message) {
    std::cout << "\033[1;32m成功: " << message << "\033[0m" << std::endl;
}

int ConsoleView::showMenu(const std::string& title, const std::vector<std::string>& options) {
    std::cout << "\n===== " << title << " =====\n";
    
    for (size_t i = 0; i < options.size(); i++) {
        std::cout << i + 1 << ". " << options[i] << std::endl;
    }
    
    std::cout << "0. 退出/返回\n";
    std::cout << "请选择: ";
    
    int choice;
    std::cin >> choice;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    
    return choice;
}

std::string ConsoleView::formatTimestamp(time_t timestamp) {
    char buffer[30];
    struct tm* timeinfo = localtime(&timestamp);
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", timeinfo);
    return std::string(buffer);
}

void ConsoleView::showTransactions(const std::vector<json>& transactions) {
    std::cout << "\n交易历史记录:\n";
    std::cout << std::setw(20) << std::left << "日期/时间" 
              << std::setw(12) << std::left << "类型"
              << std::setw(12) << std::left << "金额(¥)"
              << std::setw(15) << std::left << "交易对方"
              << std::setw(20) << std::left << "描述"
              << std::setw(12) << std::left << "余额(¥)"
              << std::endl;
    std::cout << std::string(91, '-') << std::endl;
    
    for (const auto& tx : transactions) {
        std::string typeText;
        switch (tx["type"].get<int>()) {
            case 1: typeText = "存款"; break;
            case 2: typeText = "取款"; break;
            case 3: typeText = "转入"; break;
            case 4: typeText = "转出"; break;
            default: typeText = "未知";
        }
        
        std::string dateTime = formatTimestamp(tx["timestamp"].get<time_t>());
        
        std::cout << std::setw(20) << std::left << dateTime
                  << std::setw(12) << std::left << typeText
                  << std::setw(12) << std::left << std::fixed << std::setprecision(2) << tx["amount"].get<double>()
                  << std::setw(15) << std::left << tx.value("counterparty", "")
                  << std::setw(20) << std::left << tx.value("description", "")
                  << std::setw(12) << std::left << std::fixed << std::setprecision(2) << tx["balance_after"].get<double>()
                  << std::endl;
    }
}

void ConsoleView::showDeposits(const std::vector<json>& deposits) {
    std::cout << "\n存款列表:\n";
    std::cout << std::setw(15) << std::left << "ID" 
              << std::setw(15) << std::left << "类型"
              << std::setw(15) << std::left << "金额(¥)"
              << std::setw(20) << std::left << "日期"
              << std::setw(10) << std::left << "状态"
              << std::endl;
    std::cout << std::string(75, '-') << std::endl;
    
    for (const auto& deposit : deposits) {
        std::string typeStr = deposit["type"] == 1 ? "活期存款" : "定期存款";
        std::string statusStr = deposit["isMatured"] ? "已到期" : "活跃";
        
        std::string dateStr = formatTimestamp(deposit["depositTime"].get<time_t>());
        
        std::cout << std::setw(15) << std::left << deposit["id"].get<std::string>()
                  << std::setw(15) << std::left << typeStr
                  << std::setw(15) << std::left << std::fixed << std::setprecision(2) << deposit["amount"].get<double>()
                  << std::setw(20) << std::left << dateStr
                  << std::setw(10) << std::left << statusStr
                  << std::endl;
    }
}

void ConsoleView::showDepositDetails(const json& deposit) {
    std::cout << "\n===== 存款详情 =====\n";
    std::cout << "ID: " << deposit["id"].get<std::string>() << std::endl;
    std::cout << "金额: ¥" << std::fixed << std::setprecision(2) << deposit["amount"].get<double>() << std::endl;
    
    if (deposit["type"] == 1) {
        std::cout << "类型: 活期存款 (每秒0.03%)" << std::endl;
    } else {
        std::cout << "类型: 定期存款" << std::endl;
        std::cout << "期限: " << deposit["term"].get<int>() << " 分钟" << std::endl;
        
        switch (deposit["term"].get<int>()) {
            case 2: std::cout << "利率: 每分钟0.07%" << std::endl; break;
            case 3: std::cout << "利率: 每分钟0.09%" << std::endl; break;
            case 5: std::cout << "利率: 每分钟0.1%" << std::endl; break;
        }
    }
    
    std::cout << "存款日期: " << formatTimestamp(deposit["depositTime"].get<time_t>()) << std::endl;
    
    time_t currentTime = deposit["currentTime"].get<time_t>();
    time_t elapsedSeconds = deposit["elapsedSeconds"].get<time_t>();
    int elapsedMinutes = elapsedSeconds / 60;
    int remainingSecs = elapsedSeconds % 60;
    
    std::cout << "当前时间: " << formatTimestamp(currentTime) << std::endl;
    std::cout << "已过时间: " << elapsedMinutes << " 分钟 " << remainingSecs << " 秒" << std::endl;
    
    if (deposit["type"] == 2) {  // 定期存款
        bool isMatured = deposit["isMatured"];
        std::cout << "状态: " << (isMatured ? "已到期" : "未到期") << std::endl;
        
        if (!isMatured) {
            int termSeconds = deposit["term"].get<int>() * 60;
            int remainingSeconds = termSeconds - elapsedSeconds;
            int remainingMinutes = remainingSeconds / 60;
            int remainingSecs = remainingSeconds % 60;
            std::cout << "剩余时间: " << remainingMinutes << " 分钟 " << remainingSecs << " 秒" << std::endl;
        }
    }
    
    // 显示利息计算
    if (deposit.contains("interestCalculations") && !deposit["interestCalculations"].empty()) {
        std::cout << "\n利息计算:\n";
        
        if (deposit["type"] == 1) {  // 活期存款
            std::cout << std::setw(10) << std::left << "时间(秒)" 
                      << std::setw(15) << std::left << "利息(¥)"
                      << std::setw(15) << std::left << "总额(¥)"
                      << std::endl;
        } else {  // 定期存款
            std::cout << std::setw(10) << std::left << "时间(分)" 
                      << std::setw(15) << std::left << "利息(¥)"
                      << std::setw(15) << std::left << "总额(¥)"
                      << std::endl;
        }
        
        std::cout << std::string(40, '-') << std::endl;
        
        for (const auto& calc : deposit["interestCalculations"]) {
            std::cout << std::setw(10) << std::left << calc["time"].get<int>()
                      << std::setw(15) << std::left << std::fixed << std::setprecision(2) << calc["interest"].get<double>()
                      << std::setw(15) << std::left << std::fixed << std::setprecision(2) << calc["total"].get<double>()
                      << std::endl;
        }
    }
}

void ConsoleView::showBalance(double balance) {
    std::cout << "当前余额: ¥" << std::fixed << std::setprecision(2) << balance << std::endl;
}

void ConsoleView::clearScreen() {
#if defined(_WIN32) || defined(_WIN64)
    system("cls");
#else
    system("clear");
#endif
}