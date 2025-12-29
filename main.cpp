#include <string>
#include <iostream>
#include <filesystem>

namespace fs = std::filesystem;

void handle_pwd() {
    std::cout << fs::current_path().string() << std::endl;
}

void handle_ls() {
    fs::directory_iterator dirIterator(fs::current_path());

    for (const auto& entry : dirIterator) {
                std::cout << (entry.is_directory() ? "/" : "") << entry.path().filename().string() << std::endl;
    }
}

void handle_cd(const std::string& input) {
    std::string dir = input.substr(3);
    try {
        std::filesystem::current_path(dir);
    } catch (...) {
        std::cout << "Error: " << dir << " is not exist" << std::endl;
    }
}

void handle_mkdir(const std::string& input) {
    std::string dirName = input.substr(5);
    if (fs::create_directory(dirName)) {
        std::cout << "Directory " << dirName << " created successfully." << std::endl;
    } else {
        std::cout << "Directory " << dirName << " could not be created." << std::endl;
    }
}

void handle_rm(const std::string& input) {
    std::string dirName = input.substr(3);

    if (fs::is_directory(dirName)) {
        fs::remove_all(dirName);
        std::cout << "Directory " << dirName << " deleted successfully." << std::endl;
    }  else {
        fs::remove(dirName);
        std::cout << "File " << dirName << " deleted successfully." << std::endl;
    }
}

int main() {
    std::cout << "Welcome to FileManager!" << std::endl;
    std::cout << "To leave enter \"exit\"" << std::endl;

    std::string input;

    while (true) {
        std::cout << fs::current_path().filename() << "> ";
        std::getline(std::cin, input);

        if (input == "exit") {
            std::cout << "Bye!" << std::endl;
            break;
        }

        if (input == "pwd") handle_pwd();
        if (input == "ls") handle_ls();
        if (input.substr(0, 3) == "cd ") handle_cd(input);
        if (input.substr(0, 6) == "mkdir ") handle_mkdir(input);
        if (input.substr(0, 3) == "rm ") handle_rm(input);

        std::cout << "You entered: " << input << std::endl;
    }

    return 0;
}

