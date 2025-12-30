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
    registerCommand("pwd", [this](const ParsedCommand& cmd) { printWorkingDirectory(); });
    registerCommand("ld", [this](const ParsedCommand& cmd) { listDirectory(); });
    registerCommand("cd", [this](const ParsedCommand& cmd) { changeDirectory(cmd); });
    registerCommand("mkdir", [this](const ParsedCommand& cmd) { makeDirectory(cmd); });
    registerCommand("rm", [this](const ParsedCommand& cmd) { remove(cmd); });
    registerCommand("help", [this](const ParsedCommand& cmd) { showHelp(cmd); });
}

void CommandHandler::registerCommand(const std::string &name, const std::function<void(const ParsedCommand &)> handler) {
    commands[name] = handler;
}

std::vector<std::string> CommandHandler::tokenize(const std::string &input) {
    std::vector<std::string> tokens;
    std::istringstream iss(input);
    std::string token;

    bool inQuotes = false;
    char quoteChar = '\0';
    std::string currentToken;

    for (char c : input) {
        if ((c == '\'' || c == '"') && !inQuotes) {
            inQuotes = true;
            quoteChar = c;
            if (!currentToken.empty()) {
                tokens.push_back(currentToken);
                currentToken.clear();
            }
        } else if (c == quoteChar && inQuotes) {
            inQuotes = false;
            if (!currentToken.empty()) {
                tokens.push_back(currentToken);
                currentToken.clear();
            }
        } else if (std::isspace(c) && !inQuotes) {
            if (!currentToken.empty()) {
                tokens.push_back(currentToken);
                currentToken.clear();
            }
        } else {
            currentToken += c;
        }
    }

    if (!currentToken.empty()) {
        tokens.push_back(currentToken);
    }

    return tokens;
}

void CommandHandler::parseAndExecute(const std::string &input) {
    std::vector<std::string> tokens = tokenize(input);

    if (tokens.empty()) {
        printError("No command entered");
        return;
    }

    ParsedCommand parsed = CommandParser::parse(tokens);

    if (!parsed.errors.empty()) {
        std::cerr << "Parse errors:" << std::endl;
        for (const auto& error : parsed.errors) { printError(error); }
        return;
    }

    executeParsed(parsed);
}

void CommandHandler::executeParsed(const ParsedCommand& cmd) {
    auto it = commands.find(cmd.command);
    if (it != commands.end()) {
        try {
            it->second(cmd);
        } catch (const std::exception& e) {
            std::cout << "Error executing command '" << cmd.command << "': " << e.what() << std::endl;
        }
    } else {
        std::cout << "Unknown command: " << cmd.command << std::endl;
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

void CommandHandler::changeDirectory(const ParsedCommand& cmd) {
    if (cmd.arguments.empty()) {
        std::cout << "cd: missing directory name" << std::endl;
        return;
    }

    const std::string dir = cmd.arguments[0];

    try {
        std::filesystem::current_path(dir);
    } catch (...) {
        std::cout << "cd: cannot change directory '" << dir << "'" << std::endl;
    }
}

void CommandHandler::makeDirectory(const ParsedCommand &cmd) {
    if (cmd.arguments.empty()) {
        std::cout << "mkdir: missing directory name" << std::endl;
        return;
    }

    bool createParents = cmd.flags.count("p") > 0 || cmd.flags.count("parents") > 0;
    bool verbose = cmd.flags.count("v") > 0 || cmd.flags.count("verbose") > 0;

    std::string modeStr;
    auto modeIt = cmd.options.find("m");
    if (modeIt == cmd.options.end()) modeIt = cmd.options.find("mode");

    int mode = 0755;
    if (modeIt != cmd.options.end()) {
        try {
            mode = std::stoi(modeIt->second, nullptr, 8);
        } catch (...) {
            std::cerr << "Invalid mode: " << modeIt->second << std::endl;
            return;
        }
    }

    for (const auto& dir : cmd.arguments) {
        try {
            if (createParents) {
                fs::create_directories(dir);
                if (verbose) {
                    std::cout << "mkdir: created directory '" << dir << "'" << std::endl;
                }
            } else {
                if (fs::exists(dir)) {
                    std::cout << "mkdir: directory '" << dir << "' already exists" << std::endl;
                } else {
                    fs::create_directory(dir);
                    if (verbose) {
                        std::cout << "mkdir: created directory '" << dir << "'" << std::endl;
                    }
                }
            }
        } catch (const fs::filesystem_error& e) {
            std::cout << "mkdir: cannot create directory '" << dir << "': " << e.what() << std::endl;
        }
    }
}

void CommandHandler::remove(const ParsedCommand& cmd) {
    if (cmd.arguments.empty()) {
        std::cout << "rm: missing directory name" << std::endl;
        return;
    }

    const std::string& dirName = cmd.arguments[0];

    if (fs::is_directory(dirName)) {
        fs::remove_all(dirName);
        std::cout << "Directory " << dirName << " deleted successfully." << std::endl;
    }  else {
        fs::remove(dirName);
        std::cout << "File " << dirName << " deleted successfully." << std::endl;
    }
}

void CommandHandler::showHelp(const ParsedCommand& cmd) {
    if (!cmd.arguments.empty()) {
        printUsage(cmd.arguments[0]);
    } else {
        std::cout << "\n======Available Commands======" << std::endl;
        std::cout << "pwd                             - Print current working directory" << std::endl;
        std::cout << "ld                              - Show all files and directories in current path" << std::endl;
        std::cout << "cd                              - Change working directory" << std::endl;
        std::cout << "mkdir                           - Make directory called <name>" << std::endl;
        std::cout << "rm                              - Remove directory called <name>" << std::endl;
        std::cout << "help                            - Show this help message" << std::endl;
        std::cout << "Use 'help <command>' for detailed usage of a specific command" << std::endl;
    }
}

void CommandHandler::printUsage(const std::string& command) {
    if (command == "mkdir") {
        std::cout << "\nUsage: mkdir [OPTION]... DIRECTORY..." << std::endl;
        std::cout << "Create the DIRECTORY(ies), if they do not already exist.\n" << std::endl;
        std::cout << "Options:" << std::endl;
        std::cout << "  -p, --parents     no error if existing, make parent directories as needed" << std::endl;
        std::cout << "  -m, --mode=MODE   set file mode (as in chmod), not in Windows" << std::endl;
        std::cout << "  -v, --verbose     print a message for each created directory" << std::endl;
        std::cout << "\nExamples:" << std::endl;
        std::cout << "  mkdir dir1                    Create single directory" << std::endl;
        std::cout << "  mkdir -p dir1/dir2/dir3       Create directory tree" << std::endl;
        std::cout << "  mkdir dir1 dir2 dir3          Create multiple directories" << std::endl;
        std::cout << "  mkdir -m 755 dir1             Create with specific permissions" << std::endl;
    } else {
        std::cout << "No help available for: " << command << std::endl;
    }
}

void CommandHandler::printError(const std::string& message) {
    std::cerr << "Error: " << message << std::endl;
}
