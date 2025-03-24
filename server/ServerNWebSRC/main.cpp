// main.cpp - Entry point for the banking system application (frontend functionality removed)
#include <iostream>
#include <csignal>
#include <cstring>
#include "BankingApp.h"

// Constants
const int DEFAULT_PORT = 8080;
const std::string DEFAULT_REDIS_HOST = "localhost";
const int DEFAULT_REDIS_PORT = 6379;
const std::string DEFAULT_REDIS_PASSWORD = "";

// Global application pointer for signal handling
BankingApp* globalApp = nullptr;

// Signal handler for graceful shutdown
void signalHandler(int signal) {
    std::cout << "Received signal " << signal << std::endl;
    if (globalApp) {
        globalApp->stop();
    }
    exit(signal);
}

// Print help information
void printHelp(const char* programName) {
    std::cout << "Banking System API Server\n\n";
    std::cout << "Usage: " << programName << " [options]\n\n";
    std::cout << "Options:\n";
    std::cout << "  -h, --help                  Show this help message and exit\n";
    std::cout << "  -p, --port <port>           HTTP API server port (default: " << DEFAULT_PORT << ")\n";
    std::cout << "  --redis-host <host>         Redis server host (default: " << DEFAULT_REDIS_HOST << ")\n";
    std::cout << "  --redis-port <port>         Redis server port (default: " << DEFAULT_REDIS_PORT << ")\n";
    std::cout << "  --redis-password <password> Redis server password (default: none)\n";
}

int main(int argc, char* argv[]) {
    // Default settings
    int port = DEFAULT_PORT;
    std::string redisHost = DEFAULT_REDIS_HOST;
    int redisPort = DEFAULT_REDIS_PORT;
    std::string redisPassword = DEFAULT_REDIS_PASSWORD;
    
    // Parse command line arguments
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            printHelp(argv[0]);
            return 0;
        } else if (strcmp(argv[i], "-p") == 0 || strcmp(argv[i], "--port") == 0) {
            if (i + 1 < argc) {
                port = std::stoi(argv[i + 1]);
                i++;
            } else {
                std::cerr << "Error: Port number not provided\n";
                return 1;
            }
        } else if (strcmp(argv[i], "--redis-host") == 0) {
            if (i + 1 < argc) {
                redisHost = argv[i + 1];
                i++;
            } else {
                std::cerr << "Error: Redis host not provided\n";
                return 1;
            }
        } else if (strcmp(argv[i], "--redis-port") == 0) {
            if (i + 1 < argc) {
                redisPort = std::stoi(argv[i + 1]);
                i++;
            } else {
                std::cerr << "Error: Redis port not provided\n";
                return 1;
            }
        } else if (strcmp(argv[i], "--redis-password") == 0) {
            if (i + 1 < argc) {
                redisPassword = argv[i + 1];
                i++;
            } else {
                std::cerr << "Error: Redis password not provided\n";
                return 1;
            }
        } else {
            std::cerr << "Error: Unknown option '" << argv[i] << "'\n";
            printHelp(argv[0]);
            return 1;
        }
    }

    // Setup signal handlers for graceful shutdown
    std::signal(SIGINT, signalHandler);
    std::signal(SIGTERM, signalHandler);

    try {
        // Create and run the banking application
        BankingApp app(port, redisHost, redisPort, redisPassword);
        globalApp = &app;
        
        std::cout << "Starting banking system API server...\n";
        std::cout << "API port: " << port << "\n";
        std::cout << "Redis host: " << redisHost << "\n";
        std::cout << "Redis port: " << redisPort << "\n";
        
        app.run();
        
        // The main thread will block inside app.run()
        // until a signal is received or an error occurs
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    catch (...) {
        std::cerr << "Unknown error occurred" << std::endl;
        return 1;
    }

    return 0;
}