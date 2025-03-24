#include "SessionManager.h"

// Initialize singleton instance
SessionManager SessionManager::instance;

SessionManager::SessionManager() : loggedIn(false), expiresAt(0) {
}

SessionManager& SessionManager::getInstance() {
    return instance;
}

void SessionManager::initSession(const std::string& username) {
    this->username = username;
    this->expiresAt = time(nullptr) + (2 * 60 * 60); // 2 hours expiration
    this->loggedIn = true;
}

bool SessionManager::isLoggedIn() const {
    if (!loggedIn) return false;
    return time(nullptr) < expiresAt;
}

std::string SessionManager::getCurrentUsername() const {
    return username;
}

void SessionManager::refreshSession() {
    if (loggedIn) {
        expiresAt = time(nullptr) + (2 * 60 * 60); // Extend session for 2 hours
    }
}

void SessionManager::clearSession() {
    username.clear();
    loggedIn = false;
}