// HttpServer.h - HTTP API server functionality (frontend functionality removed)
#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H

#include <string>
#include <map>
#include <mutex>
#include <thread>
#include <vector>
#include <atomic>
#include <functional>
#include "AccountManager.h"
#include "TransactionManager.h"
#include "DepositManager.h"

// HTTP handler function type
using HttpHandler = std::function<std::string(const std::map<std::string, std::string>&)>;

class HttpServer {
private:
    int port;
    int server_fd;
    std::atomic<bool> server_running;
    std::vector<std::thread> threads;
    std::map<std::string, HttpHandler> get_handlers;
    std::map<std::string, HttpHandler> post_handlers;

    // Managers
    AccountManager& accountManager;
    TransactionManager& transactionManager;
    DepositManager& depositManager;

    // Helper methods
    void handleClient(int client_socket);
    void parseRequest(const std::string& request, std::string& method, std::string& path, std::map<std::string, std::string>& params);
    void sendResponse(int client_socket, const std::string& content_type, const std::string& content);

    // Register all API route handlers
    void registerHandlers();

public:
    HttpServer(int port,
        AccountManager& am, TransactionManager& tm, DepositManager& dm);
    ~HttpServer();

    // Start the server
    bool start();

    // Stop the server
    void stop();
};

#endif // HTTP_SERVER_H