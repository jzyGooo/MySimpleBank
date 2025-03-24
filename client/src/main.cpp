#include "BankingClient.h"
#include "ApiClient.h"
#include <iostream>
#include <cstring>

// Default server URL
const std::string DEFAULT_SERVER_URL = "http://localhost:8080";

// Print help information
void printHelp(const char* programName) {
    std::cout << "Banking System CLI Client\n\n";
    std::cout << "Usage: " << programName << " [options]\n\n";
    std::cout << "Options:\n";
    std::cout << "  -h, --help           Show this help message and exit\n";
    std::cout << "  -s, --server <url>   Specify server URL (default: " << DEFAULT_SERVER_URL << ")\n";
}

int main(int argc, char* argv[]) {
    std::string serverUrl = DEFAULT_SERVER_URL;
    
    // Parse command line arguments
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            printHelp(argv[0]);
            return 0;
        } else if (strcmp(argv[i], "-s") == 0 || strcmp(argv[i], "--server") == 0) {
            if (i + 1 < argc) {
                serverUrl = argv[i + 1];
                i++;
            } else {
                std::cerr << "Error: Server URL not provided\n";
                return 1;
            }
        } else {
            std::cerr << "Error: Unknown option '" << argv[i] << "'\n";
            printHelp(argv[0]);
            return 1;
        }
    }
    
    // Initialize CURL
    if (!ApiClient::initialize()) {
        std::cerr << "Failed to initialize CURL library\n";
        return 1;
    }
    
    try {
        // Create and run the client
        BankingClient client(serverUrl);
        client.run();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        ApiClient::cleanup();
        return 1;
    }
    
    // Clean up CURL
    ApiClient::cleanup();
    
    return 0;
}