#include "InputHandler.h"
#include <iostream>
#include <limits>

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#else
#include <termios.h>
#include <unistd.h>
#endif

std::string InputHandler::readLine() {
    std::string line;
    std::getline(std::cin, line);
    return line;
}

std::string InputHandler::readPassword() {
    std::string password;
    
#if defined(_WIN32) || defined(_WIN64)
    // Windows implementation
    HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
    DWORD mode = 0;
    GetConsoleMode(hStdin, &mode);
    SetConsoleMode(hStdin, mode & (~ENABLE_ECHO_INPUT));
    
    std::getline(std::cin, password);
    
    SetConsoleMode(hStdin, mode);
#else
    // UNIX implementation
    termios oldt, newt;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~ECHO;
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    
    std::getline(std::cin, password);
    
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
#endif
    
    std::cout << std::endl; // Add a newline after password input
    return password;
}

int InputHandler::readInt() {
    int value;
    std::cin >> value;
    clearInputBuffer();
    return value;
}

double InputHandler::readDouble() {
    double value;
    std::cin >> value;
    clearInputBuffer();
    return value;
}

void InputHandler::clearInputBuffer() {
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

bool InputHandler::confirm(const std::string& message) {
    std::cout << message << " (y/n): ";
    std::string response = readLine();
    return (response == "y" || response == "Y");
}