#include "ApiClient.h"
#include <sstream>

// Static callback function for CURL
size_t ApiClient::WriteCallback(void *contents, size_t size, size_t nmemb, void *userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

ApiClient::ApiClient(const std::string& baseUrl) : baseUrl(baseUrl) {
}

ApiClient::~ApiClient() {
}

bool ApiClient::initialize() {
    return curl_global_init(CURL_GLOBAL_ALL) == CURLE_OK;
}

void ApiClient::cleanup() {
    curl_global_cleanup();
}

bool ApiClient::get(const std::string& endpoint, 
                    const std::map<std::string, std::string>& params, 
                    json& responseJson) {
    CURL* curl = curl_easy_init();
    if (!curl) {
        lastError = "Failed to initialize CURL";
        return false;
    }
    
    // Build URL with query parameters
    std::string url = baseUrl + endpoint;
    if (!params.empty()) {
        url += "?";
        bool first = true;
        for (const auto& param : params) {
            if (!first) url += "&";
            first = false;
            
            char* escapedKey = curl_easy_escape(curl, param.first.c_str(), param.first.length());
            char* escapedValue = curl_easy_escape(curl, param.second.c_str(), param.second.length());
            
            url += std::string(escapedKey) + "=" + std::string(escapedValue);
            
            curl_free(escapedKey);
            curl_free(escapedValue);
        }
    }
    
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    
    // Response data
    std::string responseData;
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseData);
    
    // Execute request
    CURLcode res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);
    
    if (res != CURLE_OK) {
        lastError = "CURL request failed: " + std::string(curl_easy_strerror(res));
        return false;
    }
    
    // Parse JSON response
    try {
        responseJson = json::parse(responseData);
        return true;
    } catch (const json::parse_error& e) {
        lastError = "JSON parse error: " + std::string(e.what());
        return false;
    }
}

bool ApiClient::post(const std::string& endpoint, 
                     const std::map<std::string, std::string>& params, 
                     json& responseJson) {
    CURL* curl = curl_easy_init();
    if (!curl) {
        lastError = "Failed to initialize CURL";
        return false;
    }
    
    // Build URL
    std::string url = baseUrl + endpoint;
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    
    // Build POST data
    std::string postData;
    bool first = true;
    for (const auto& param : params) {
        if (!first) postData += "&";
        first = false;
        
        char* escapedKey = curl_easy_escape(curl, param.first.c_str(), param.first.length());
        char* escapedValue = curl_easy_escape(curl, param.second.c_str(), param.second.length());
        
        postData += std::string(escapedKey) + "=" + std::string(escapedValue);
        
        curl_free(escapedKey);
        curl_free(escapedValue);
    }
    
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postData.c_str());
    
    // Response data
    std::string responseData;
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseData);
    
    // Execute request
    CURLcode res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);
    
    if (res != CURLE_OK) {
        lastError = "CURL request failed: " + std::string(curl_easy_strerror(res));
        return false;
    }
    
    // Parse JSON response
    try {
        responseJson = json::parse(responseData);
        return true;
    } catch (const json::parse_error& e) {
        lastError = "JSON parse error: " + std::string(e.what());
        return false;
    }
}

std::string ApiClient::getLastError() const {
    return lastError;
}