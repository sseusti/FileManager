#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <map>
#include <functional>
#include <filesystem>

#include "CommandHandler.h"

namespace fs = std::filesystem;

CommandHandler::CommandHandler() {
    commands["pwd"] = [this](const std::vector<std::string>& args) {
        printWorkingDirectory();
    };

    commands["ld"] = [this](const std::vector<std::string>& args) {
        listDirectory();
    };

    commands["cd"] = [this](const std::vector<std::string>& args) {
        changeDirectory(args);
    };

    commands["mkdir"] = [this](const std::vector<std::string>& args) {
        makeDirectory(args);
    };

    commands["rm"] = [this](const std::vector<std::string>& args) {
        remove(args);
    };
}

std::vector<std::string> CommandHandler::parseInput(const std::string& input) {
    std::vector<std::string> tokens;
    std::istringstream iss(input);
    std::string token;

    while (iss >> token) {
        tokens.push_back(token);
    }

    return tokens;
}

void CommandHandler::parseCommands(const std::string &input) {
    if (input.empty()) {
        return;
    }

    std::vector<std::string> tokens = parseInput(input);

    if (tokens.empty()) {
        printError("Empty input");
        return;
    }

    std::string command = tokens[0];
    std::transform(command.begin(), command.end(), command.begin(), ::tolower);

    std::vector<std::string> args(tokens.begin() + 1, tokens.end());

    executeCommands(command, args);
}

void CommandHandler::executeCommands(const std::string& cmd, const std::vector<std::string>& args) {
    std::string lowerCmd = cmd;
    std::transform(lowerCmd.begin(), lowerCmd.end(), lowerCmd.begin(), ::tolower);

    auto it = commands.find(lowerCmd);
    if (it != commands.end()) {
        try {
            it->second(args);
        } catch (const std::exception& e) {
            printError("Error executing command '" + cmd + "': " + e.what());
        }
    } else {
        printError("Unknown command: " + cmd);
        std::cout << "Type 'help' for available commands" << std::endl;
    }
}

void CommandHandler::printWorkingDirectory() {
     std::cout << fs::current_path().string() << std::endl;
}

void CommandHandler::listDirectory() {
    fs::directory_iterator dirIterator(fs::current_path());

    for (const auto& entry : dirIterator) {
        std::cout << (entry.is_directory() ? "/" : "") << entry.path().filename().string() << std::endl;
    }
}

void CommandHandler::changeDirectory(const std::vector<std::string> &args) {
    if (!validateArguments(args, 1, "cd")) {
        std::cout << "Usage: cd <name>" << std::endl;
        return;
    }

    const std::string& dir = args[0];

    try {
        std::filesystem::current_path(dir);
    } catch (...) {
        std::cout << "Error: " << dir << " is not exist" << std::endl;
    }
}

void CommandHandler::makeDirectory(const std::vector<std::string>& args) {
    if (!validateArguments(args, 1, "cd")) {
        std::cout << "Usage: mkdir <name>" << std::endl;
        return;
    }

    const std::string& dirName = args[0];

    if (fs::create_directory(dirName)) {
        std::cout << "Directory " << dirName << " created successfully." << std::endl;
    } else {
        std::cout << "Directory " << dirName << " could not be created." << std::endl;
    }
}

void CommandHandler::remove(const std::vector<std::string> &args) {
    if (!validateArguments(args, 1, "cd")) {
        std::cout << "Usage: rm <name>" << std::endl;
        return;
    }

    const std::string& dirName = args[0];

    if (fs::is_directory(dirName)) {
        fs::remove_all(dirName);
        std::cout << "Directory " << dirName << " deleted successfully." << std::endl;
    }  else {
        fs::remove(dirName);
        std::cout << "File " << dirName << " deleted successfully." << std::endl;
    }
}

bool CommandHandler::validateArguments(const std::vector<std::string>& args, size_t expected, const std::string& command) {
    if (args.size() >= expected) {
        return true;
    }

    std::cout << "Error: Command '" << command << "' requires at least "
              << expected << " argument(s)" << std::endl;
    std::cout << "Received " << args.size() << " argument(s)" << std::endl;

    return false;
}

void CommandHandler::printError(const std::string& message) {
    std::cerr << "Error: " << message << std::endl;
}
