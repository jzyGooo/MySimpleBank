#ifndef INPUT_HANDLER_H
#define INPUT_HANDLER_H

#include <string>

class InputHandler {
public:
    // Read a line of text
    static std::string readLine();

    // Read a password (without echoing)
    static std::string readPassword();

    // Read an integer
    static int readInt();

    // Read a double
    static double readDouble();

    // Clear input buffer
    static void clearInputBuffer();

    // Ask for confirmation (y/n)
    static bool confirm(const std::string& message);
};

#endif // INPUT_HANDLER_H