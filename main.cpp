#include <string>
#include <iostream>
#include <filesystem>

namespace fs = std::filesystem;

int main() {
    std::cout << "Welcome to FileManager!" << std::endl;
    std::cout << "To leave enter \"exit\"" << std::endl;

    std::string input;

    while (true) {
        std::cout << fs::current_path().filename() << "> ";
        std::cin >> input;

        if (input == "exit") {
            std::cout << "Bye!" << std::endl;
            break;
        }

        if (input == "pwd") {
            std::cout << fs::current_path() << std::endl;
        }

        if (input == "ls") {
            fs::directory_iterator dirIterator(fs::current_path());

            for (const auto& entry : dirIterator) {
                std::cout << (entry.is_directory() ? "directory " : "file ") << entry.path().filename().string() << std::endl;
            }
        }

        if (input.substr(0, 3) == "cd ") {
            std::string dir = input.substr(3);
            fs::current_path(dir);
        }

        std::cout << "You entered: " << input << std::endl;
    }

    return 0;
}