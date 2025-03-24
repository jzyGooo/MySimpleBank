#ifndef API_CLIENT_H
#define API_CLIENT_H

#include <string>
#include <map>
#include <curl/curl.h>
#include "../extern/json.hpp"

using json = nlohmann::json;

class ApiClient {
private:
    std::string baseUrl;
    std::string lastError;

    // Callback for CURL to write response data
    static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp);

public:
    ApiClient(const std::string& baseUrl);
    ~ApiClient();

    // Initialize and cleanup CURL globally
    static bool initialize();
    static void cleanup();

    // HTTP requests
    bool get(const std::string& endpoint,
        const std::map<std::string, std::string>& params,
        json& responseJson);

    bool post(const std::string& endpoint,
        const std::map<std::string, std::string>& params,
        json& responseJson);

    // Error handling
    std::string getLastError() const;
};

#endif // API_CLIENT_H