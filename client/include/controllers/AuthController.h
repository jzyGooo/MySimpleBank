#ifndef AUTH_CONTROLLER_H
#define AUTH_CONTROLLER_H

#include "ApiClient.h"
#include "SessionManager.h"

class AuthController {
private:
    ApiClient& apiClient;
    SessionManager& sessionManager;

public:
    AuthController(ApiClient& apiClient, SessionManager& sessionManager);

    // Login
    bool login();

    // Register
    bool registerUser();

    // Logout
    void logout();
};

#endif // AUTH_CONTROLLER_H