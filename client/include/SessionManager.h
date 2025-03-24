#ifndef SESSION_MANAGER_H
#define SESSION_MANAGER_H

#include <string>
#include <ctime>

class SessionManager {
private:
    std::string username;
    time_t expiresAt;
    bool loggedIn;

    // Singleton pattern
    static SessionManager instance;
    SessionManager();

public:
    static SessionManager& getInstance();

    void initSession(const std::string& username);
    bool isLoggedIn() const;
    std::string getCurrentUsername() const;
    void refreshSession();
    void clearSession();
};

#endif // SESSION_MANAGER_H