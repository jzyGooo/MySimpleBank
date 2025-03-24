// HttpServer.cpp - Implementation of HTTP API server functionality (frontend functionality removed)
#include "HttpServer.h"
#include <iostream>
#include <sstream>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

// Maximum number of client connections
const int MAX_CONNECTIONS = 100;

HttpServer::HttpServer(int port, 
                     AccountManager& am, TransactionManager& tm, DepositManager& dm)
    : port(port), server_running(false),
      accountManager(am), transactionManager(tm), depositManager(dm) {
    // Register all API route handlers
    registerHandlers();
}

HttpServer::~HttpServer() {
    stop();
}

bool HttpServer::start() {
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    // Create socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        std::cerr << "Socket creation failed" << std::endl;
        return false;
    }

    // Set socket options to allow port reuse
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        std::cerr << "Setsockopt failed" << std::endl;
        return false;
    }

    // Setup address structure
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    // Bind socket
    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        std::cerr << "Bind failed" << std::endl;
        return false;
    }

    // Listen for connections
    if (listen(server_fd, MAX_CONNECTIONS) < 0) {
        std::cerr << "Listen failed" << std::endl;
        return false;
    }

    std::cout << "API Server started on port " << port << std::endl;

    server_running = true;

    // Main loop to accept client connections
    while (server_running) {
        int client_socket;
        if ((client_socket = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen)) < 0) {
            if (!server_running) {
                break;  // Server was stopped
            }
            std::cerr << "Accept failed" << std::endl;
            continue;
        }

        // Create new thread to handle client connection
        threads.push_back(std::thread(&HttpServer::handleClient, this, client_socket));
    }

    return true;
}

void HttpServer::stop() {
    if (server_running) {
        server_running = false;
        close(server_fd);  // Close server socket to unblock accept()

        // Wait for all client threads to finish
        for (auto& thread : threads) {
            if (thread.joinable()) {
                thread.join();
            }
        }
        threads.clear();
    }
}

void HttpServer::handleClient(int client_socket) {
    char buffer[4096] = { 0 };
    ssize_t bytes_read = read(client_socket, buffer, 4096);

    if (bytes_read > 0) {
        std::string request(buffer);
        std::string method, path;
        std::map<std::string, std::string> params;

        parseRequest(request, method, path, params);

        // API routes
        if (method == "GET" && get_handlers.find(path) != get_handlers.end()) {
            std::string response = get_handlers[path](params);
            sendResponse(client_socket, "application/json", response);
        }
        else if (method == "POST" && post_handlers.find(path) != post_handlers.end()) {
            std::string response = post_handlers[path](params);
            sendResponse(client_socket, "application/json", response);
        }
        else {
            // 404 Not Found
            sendResponse(client_socket, "application/json", "{\"status\":\"error\",\"message\":\"API endpoint not found\"}");
        }
    }

    // Close client connection
    close(client_socket);
}

void HttpServer::parseRequest(const std::string& request, std::string& method, std::string& path, std::map<std::string, std::string>& params) {
    std::istringstream iss(request);
    std::string line;

    // Get request method and path
    std::getline(iss, line);
    std::istringstream line_iss(line);
    line_iss >> method >> path;

    // Check if path contains query parameters
    size_t query_pos = path.find('?');
    if (query_pos != std::string::npos) {
        std::string query_string = path.substr(query_pos + 1);
        path = path.substr(0, query_pos);

        // Parse query parameters
        size_t pos = 0;
        while ((pos = query_string.find('&')) != std::string::npos) {
            std::string param = query_string.substr(0, pos);
            size_t eq_pos = param.find('=');
            if (eq_pos != std::string::npos) {
                params[param.substr(0, eq_pos)] = param.substr(eq_pos + 1);
            }
            query_string.erase(0, pos + 1);
        }

        size_t eq_pos = query_string.find('=');
        if (eq_pos != std::string::npos) {
            params[query_string.substr(0, eq_pos)] = query_string.substr(eq_pos + 1);
        }
    }

    // If POST request, parse request body
    if (method == "POST") {
        // Skip request headers
        while (std::getline(iss, line) && !line.empty() && line != "\r") {}

        // Read request body
        std::string body;
        std::getline(iss, body);

        // Parse body parameters
        size_t pos = 0;
        while ((pos = body.find('&')) != std::string::npos) {
            std::string param = body.substr(0, pos);
            size_t eq_pos = param.find('=');
            if (eq_pos != std::string::npos) {
                params[param.substr(0, eq_pos)] = param.substr(eq_pos + 1);
            }
            body.erase(0, pos + 1);
        }

        size_t eq_pos = body.find('=');
        if (eq_pos != std::string::npos) {
            params[body.substr(0, eq_pos)] = body.substr(eq_pos + 1);
        }
    }
}

void HttpServer::sendResponse(int client_socket, const std::string& content_type, const std::string& content) {
    std::string response = "HTTP/1.1 200 OK\r\n";
    response += "Content-Type: " + content_type + "\r\n";
    response += "Content-Length: " + std::to_string(content.length()) + "\r\n";
    response += "Access-Control-Allow-Origin: *\r\n"; // Enable CORS for API
    response += "Connection: close\r\n";
    response += "\r\n";
    response += content;

    send(client_socket, response.c_str(), response.length(), 0);
}

void HttpServer::registerHandlers() {
    // Register POST handlers
    post_handlers["/api/register"] = [this](const std::map<std::string, std::string>& params) -> std::string {
        std::string username = params.at("username");
        std::string password = params.at("password");
        int account_type = std::stoi(params.at("account_type"));

        bool success = accountManager.registerUser(username, password, account_type);

        if (success) {
            return "{\"status\":\"success\",\"message\":\"Registration successful\"}";
        }
        else {
            return "{\"status\":\"error\",\"message\":\"Username already exists\"}";
        }
    };

    post_handlers["/api/login"] = [this](const std::map<std::string, std::string>& params) -> std::string {
        std::string username = params.at("username");
        std::string password = params.at("password");

        bool success = accountManager.authenticateUser(username, password);

        if (success) {
            return "{\"status\":\"success\",\"message\":\"Login successful\",\"username\":\"" + username + "\"}";
        }
        else {
            return "{\"status\":\"error\",\"message\":\"Invalid username or password\"}";
        }
    };

    post_handlers["/api/deposit"] = [this](const std::map<std::string, std::string>& params) -> std::string {
        std::string username = params.at("username");
        double amount = std::stod(params.at("amount"));

        bool success = transactionManager.deposit(username, amount);
        double new_balance = transactionManager.getBalance(username);

        if (success) {
            return "{\"status\":\"success\",\"message\":\"Deposit successful\",\"balance\":" + std::to_string(new_balance) + "}";
        }
        else {
            return "{\"status\":\"error\",\"message\":\"Deposit failed\"}";
        }
    };

    post_handlers["/api/withdraw"] = [this](const std::map<std::string, std::string>& params) -> std::string {
        std::string username = params.at("username");
        double amount = std::stod(params.at("amount"));

        bool success = transactionManager.withdraw(username, amount);
        double new_balance = transactionManager.getBalance(username);

        if (success) {
            return "{\"status\":\"success\",\"message\":\"Withdrawal successful\",\"balance\":" + std::to_string(new_balance) + "}";
        }
        else {
            return "{\"status\":\"error\",\"message\":\"Withdrawal failed. Insufficient funds or invalid amount\"}";
        }
    };

    post_handlers["/api/transfer"] = [this](const std::map<std::string, std::string>& params) -> std::string {
        std::string from_username = params.at("username");
        std::string to_username = params.at("to_username");
        double amount = std::stod(params.at("amount"));

        bool success = transactionManager.transfer(from_username, to_username, amount);
        double new_balance = transactionManager.getBalance(from_username);

        if (success) {
            return "{\"status\":\"success\",\"message\":\"Transfer successful\",\"balance\":" + std::to_string(new_balance) + "}";
        }
        else {
            return "{\"status\":\"error\",\"message\":\"Transfer failed. Check recipient username, amount, and your balance\"}";
        }
    };

    post_handlers["/api/create-deposit"] = [this](const std::map<std::string, std::string>& params) -> std::string {
        std::string username = params.at("username");
        double amount = std::stod(params.at("amount"));
        int deposit_type = std::stoi(params.at("deposit_type"));
        int deposit_term = 0;

        if (deposit_type == TIME_DEPOSIT) {
            deposit_term = std::stoi(params.at("deposit_term"));
        }

        bool success = depositManager.createDeposit(username, amount, deposit_type, deposit_term);
        double new_balance = transactionManager.getBalance(username);

        if (success) {
            return "{\"status\":\"success\",\"message\":\"Deposit created successfully\",\"balance\":" + std::to_string(new_balance) + "}";
        }
        else {
            return "{\"status\":\"error\",\"message\":\"Failed to create deposit. Please check your balance and input.\"}";
        }
    };

    post_handlers["/api/withdraw-deposit"] = [this](const std::map<std::string, std::string>& params) -> std::string {
        std::string username = params.at("username");
        std::string deposit_id = params.at("deposit_id");  // 使用字符串格式的ID
        double amount = std::stod(params.at("amount"));
    
        bool success = depositManager.withdrawDeposit(username, deposit_id, amount);
        double new_balance = transactionManager.getBalance(username);
    
        if (success) {
            return "{\"status\":\"success\",\"message\":\"Withdrawal successful\",\"balance\":" + std::to_string(new_balance) + "}";
        }
        else {
            return "{\"status\":\"error\",\"message\":\"Withdrawal failed. Check if the deposit exists, the amount is valid, or if time deposit has matured.\"}";
        }
    };

    // Register GET handlers
    get_handlers["/api/balance"] = [this](const std::map<std::string, std::string>& params) -> std::string {
        std::string username = params.at("username");
        double balance = transactionManager.getBalance(username);

        if (balance >= 0) {
            return "{\"status\":\"success\",\"balance\":" + std::to_string(balance) + "}";
        }
        else {
            return "{\"status\":\"error\",\"message\":\"User not found\"}";
        }
    };

    get_handlers["/api/get-deposits"] = [this](const std::map<std::string, std::string>& params) -> std::string {
        std::string username = params.at("username");
        std::vector<Deposit> deposits = depositManager.getUserDeposits(username);
    
        std::string deposits_json = "[";
        for (size_t i = 0; i < deposits.size(); i++) {
            const auto& deposit = deposits[i];
    
            deposits_json += "{";
            deposits_json += "\"id\":\"" + deposit.id + "\",";  // 使用字符串格式的ID
            deposits_json += "\"amount\":" + std::to_string(deposit.amount) + ",";
            deposits_json += "\"type\":" + std::to_string(deposit.type) + ",";
    
            if (deposit.type == TIME_DEPOSIT) {
                deposits_json += "\"term\":" + std::to_string(deposit.term) + ",";
            }
    
            deposits_json += "\"depositTime\":" + std::to_string(deposit.depositTime) + ",";
            deposits_json += "\"currentTime\":" + std::to_string(time(nullptr)) + ",";
            deposits_json += "\"isMatured\":" + std::string(deposit.isMatured ? "true" : "false");
            deposits_json += "}";
    
            if (i < deposits.size() - 1) {
                deposits_json += ",";
            }
        }
        deposits_json += "]";
    
        return "{\"status\":\"success\",\"deposits\":" + deposits_json + "}";
    };

    get_handlers["/api/get-deposit-details"] = [this](const std::map<std::string, std::string>& params) -> std::string {
        try {
            // 检查必需参数是否存在
            if (params.find("username") == params.end() || params.find("deposit_id") == params.end()) {
                return "{\"status\":\"error\",\"message\":\"Missing required parameters\"}";
            }
            
            std::string username = params.at("username");
            std::string deposit_id = params.at("deposit_id");
            
            // 调试信息
            std::cout << "API调用: get-deposit-details, username=" << username << ", deposit_id=" << deposit_id << std::endl;
    
            Deposit deposit = depositManager.getDepositDetails(username, deposit_id);
            time_t current_time = time(nullptr);
    
            if (!deposit.id.empty()) {  // 检查ID是否为空
                std::string deposit_json = "{";
                deposit_json += "\"id\":\"" + deposit.id + "\",";  // 使用字符串格式的ID
                deposit_json += "\"amount\":" + std::to_string(deposit.amount) + ",";
                deposit_json += "\"type\":" + std::to_string(deposit.type) + ",";
    
                if (deposit.type == TIME_DEPOSIT) {
                    deposit_json += "\"term\":" + std::to_string(deposit.term) + ",";
                }
    
                deposit_json += "\"depositTime\":" + std::to_string(deposit.depositTime) + ",";
                deposit_json += "\"currentTime\":" + std::to_string(current_time) + ",";
    
                // 计算经过的时间
                time_t elapsed_seconds = current_time - deposit.depositTime;
                deposit_json += "\"elapsedSeconds\":" + std::to_string(elapsed_seconds) + ",";
    
                // 检查定期存款是否到期
                bool is_matured = false;
                if (deposit.type == TIME_DEPOSIT) {
                    is_matured = elapsed_seconds >= deposit.term * 60;
                }
                deposit_json += "\"isMatured\":" + std::string(is_matured ? "true" : "false") + ",";
    
                // 计算不同时间点的利息
                std::string interest_json = "[";
    
                if (deposit.type == DEMAND_DEPOSIT) {
                    // 活期存款: 30秒, 60秒, 90秒, 120秒
                    int time_points[] = { 30, 60, 90, 120 };
    
                    for (size_t i = 0; i < 4; i++) {
                        double interest = depositManager.calculateInterest(deposit, time_points[i]);
    
                        interest_json += "{";
                        interest_json += "\"time\":" + std::to_string(time_points[i]) + ",";
                        interest_json += "\"interest\":" + std::to_string(interest) + ",";
                        interest_json += "\"total\":" + std::to_string(deposit.amount + interest);
                        interest_json += "}";
    
                        if (i < 3) {
                            interest_json += ",";
                        }
                    }
                }
                else if (deposit.type == TIME_DEPOSIT) {
                    // 定期存款: 3分钟, 6分钟, 9分钟, 12分钟
                    int time_points[] = { 3, 6, 9, 12 };
    
                    for (size_t i = 0; i < 4; i++) {
                        double interest = depositManager.calculateInterest(deposit, time_points[i] * 60);
    
                        interest_json += "{";
                        interest_json += "\"time\":" + std::to_string(time_points[i]) + ",";
                        interest_json += "\"interest\":" + std::to_string(interest) + ",";
                        interest_json += "\"total\":" + std::to_string(deposit.amount + interest);
                        interest_json += "}";
    
                        if (i < 3) {
                            interest_json += ",";
                        }
                    }
                }
    
                interest_json += "]";
                deposit_json += "\"interestCalculations\":" + interest_json;
                deposit_json += "}";
    
                std::string response = "{\"status\":\"success\",\"deposit\":" + deposit_json + "}";
                std::cout << "API响应: 成功返回存款详情, id=" << deposit.id << std::endl;
                return response;
            }
            else {
                std::cout << "API错误: 未找到存款, username=" << username << ", deposit_id=" << deposit_id << std::endl;
                return "{\"status\":\"error\",\"message\":\"Deposit not found\"}";
            }
        }
        catch (const std::exception& e) {
            std::cerr << "API异常: " << e.what() << std::endl;
            return "{\"status\":\"error\",\"message\":\"Server error: " + std::string(e.what()) + "\"}";
        }
        catch (...) {
            std::cerr << "API异常: 未知错误" << std::endl;
            return "{\"status\":\"error\",\"message\":\"Unknown server error\"}";
        }
    };

    get_handlers["/api/transaction-history"] = [this](const std::map<std::string, std::string>& params) -> std::string {
        try {
            std::string username = params.at("username");
            std::vector<TransactionRecord> transactions = transactionManager.getTransactionHistory(username);
            
            std::string transactions_json = "[";
            for (size_t i = 0; i < transactions.size(); i++) {
                const auto& tx = transactions[i];
                
                transactions_json += "{";
                transactions_json += "\"id\":\"" + tx.id + "\",";
                transactions_json += "\"type\":" + std::to_string(tx.type) + ",";
                transactions_json += "\"amount\":" + std::to_string(tx.amount) + ",";
                transactions_json += "\"balance_after\":" + std::to_string(tx.balance_after) + ",";
                
                // 对交易对方进行特殊处理，避免JSON语法错误
                if (!tx.counterparty.empty()) {
                    transactions_json += "\"counterparty\":\"" + tx.counterparty + "\",";
                } else {
                    transactions_json += "\"counterparty\":\"\",";
                }
                
                // 对描述进行特殊处理，避免JSON语法错误
                if (!tx.description.empty()) {
                    transactions_json += "\"description\":\"" + tx.description + "\",";
                } else {
                    transactions_json += "\"description\":\"\",";
                }
                
                transactions_json += "\"timestamp\":" + std::to_string(tx.timestamp);
                transactions_json += "}";
                
                if (i < transactions.size() - 1) {
                    transactions_json += ",";
                }
            }
            transactions_json += "]";
            
            return "{\"status\":\"success\",\"transactions\":" + transactions_json + "}";
        }
        catch (const std::exception& e) {
            return "{\"status\":\"error\",\"message\":\"" + std::string(e.what()) + "\"}";
        }
    };
}