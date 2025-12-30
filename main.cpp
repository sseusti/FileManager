#include <string>
#include <iostream>
#include <filesystem>
#include <sstream>

#include "src/CommandHandler.h"

namespace fs = std::filesystem;

int main() {
    std::cout << "Welcome to FileManager!" << std::endl;
    std::cout << "To leave enter \"exit\"" << std::endl;

    std::string input;
    CommandHandler commandHandler;

    while (true) {
        std::cout << fs::current_path().filename() << "> ";
        std::getline(std::cin, input);

        if (input == "exit") {
            std::cout << "Bye!" << std::endl;
            break;
        }
        if (input.empty()) continue;

        commandHandler.parseCommands(input);
        std::cout << "You entered: " << input << std::endl;
    }

    return 0;
}

