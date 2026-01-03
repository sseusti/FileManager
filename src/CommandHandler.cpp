#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <map>
#include <functional>
#include <filesystem>
#include <fstream>
#include <format>
#include <termcolor/termcolor.hpp>

#include "CommandHandler.h"

namespace fs = std::filesystem;

CommandHandler::CommandHandler() {
    registerCommand("pwd", [this](const ParsedCommand& cmd) { printWorkingDirectory(); });
    registerCommand("ld", [this](const ParsedCommand& cmd) { listDirectory(cmd); });
    registerCommand("cd", [this](const ParsedCommand& cmd) { changeDirectory(cmd); });
    registerCommand("mkdir", [this](const ParsedCommand& cmd) { makeDirectory(cmd); });
    registerCommand("rm", [this](const ParsedCommand& cmd) { remove(cmd); });
    registerCommand("help", [this](const ParsedCommand& cmd) { showHelp(cmd); });
    registerCommand("touch", [this](const ParsedCommand& cmd) { touch(cmd); });
}

void CommandHandler::registerCommand(const std::string &name, const std::function<void(const ParsedCommand &)>& handler) {
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
    const std::string workingDirectory = fs::current_path().string();
    printMessage(workingDirectory);
}

void CommandHandler::listDirectory(const ParsedCommand& cmd) {
    bool longFormat = cmd.flags.count("l") > 0 || cmd.flags.count("long") > 0;

    fs::directory_iterator dirIterator(fs::current_path());

    for (const auto& entry : dirIterator) {
        if (!longFormat) {
            std::string message = (entry.is_directory() ? "/" : (entry.is_symlink() ? "@" : "*")) + entry.path().filename().string();
            printMessage(message);

        } else {
            std::string type = entry.path().extension().string();
            std::string file = entry.path().filename().string();

            char type_char = '-';
            if (entry.is_directory()) type_char = 'd';
            else if (entry.is_symlink()) type_char = 'l';

            std::uintmax_t size = 0;
            std::string size_str = "0";

            try {
                if (entry.is_regular_file() && !entry.is_symlink()) {
                    size = fs::file_size(entry.path());

                    if (size < 1024) {
                        size_str = std::to_string(size) + " B";
                    } else if (size < 1024 * 1024) {
                        size_str = std::to_string(size / 1024) + " KB";
                    } else if (size < 1024 * 1024 * 1024) {
                        size_str = std::to_string(size / (1024 * 1024)) + " MB";
                    } else {
                        size_str = std::to_string(size / (1024 * 1024 * 1024)) + " GB";
                    }
                }
            } catch (const std::exception& e) {
                size_str = "N/A";
            }

            std::string time_str;
            try {
                auto ftime = fs::last_write_time(entry.path());
                auto sctp = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
                    ftime - fs::file_time_type::clock::now() + std::chrono::system_clock::now()
                );
                std::time_t cftime = std::chrono::system_clock::to_time_t(sctp);
                time_str = std::ctime(&cftime);
                time_str.pop_back();
            } catch (...) {
                time_str = "N/A";
            }

            std::string message = std::format("{}{} {:>10} {:>20} {}",
                type_char,
                (entry.is_symlink() ? "l" : (entry.is_directory() ? "d" : "-")),
                size_str,
                time_str,
                file);
            printMessage(message);
        }
    }
}

void CommandHandler::changeDirectory(const ParsedCommand& cmd) {
    if (cmd.arguments.empty()) {
        printError("cd: missing directory name");
        printUsage("cd");
        return;
    }

    const std::string dir = cmd.arguments[0];

    try {
        std::filesystem::current_path(dir);
    } catch (...) {
        std::string message = std::format("cd: cannot change directory '{}'", dir);
        printError(message);
    }
}

void CommandHandler::touch(const ParsedCommand &cmd) {
    if (cmd.arguments.empty()) {
        printError("touch: missing file name");
        printUsage("touch");
        return;
    }

    bool verbose = cmd.flags.count("v") > 0 || cmd.flags.count("verbose") > 0;
    bool force = cmd.flags.count("f") > 0 || cmd.flags.count("force") > 0;
    bool interactive = cmd.flags.count("i") > 0 || cmd.flags.count("interactive") > 0;
    const std::string file = cmd.arguments[0];
    std::string message;

    if (fs::exists(file)) {
        if (!force) {
            message = std::format("touch: file '{}' already exists", file);
            printMessage(message);

            if (confirmChange(file, interactive)) {
                removeFile(file, true, false);
                std::ofstream outputFile(file);
                if (verbose) {
                    message = std::format("touch: rewrote file '{}'", file);
                    printMessage(message);
                }
            } else {
                return;
            }
        } else {
            removeFile(file, true, false);
            std::ofstream outputFile(file);
            if (verbose) {
                message = std::format("touch: rewrote file '{}'", file);
                printMessage(message);
            }
        }
    } else {
        std::ofstream outputFile(file);
        if (verbose) {
            message = std::format("touch: rewrote file '{}'", file);
            printMessage(message);
        }
    }
}

void CommandHandler::makeDirectory(const ParsedCommand &cmd) {
    if (cmd.arguments.empty()) {
        printError("mkdir: missing directory name");
        printUsage("mkdir");
        return;
    }

    bool createParents = cmd.flags.count("p") > 0 || cmd.flags.count("parents") > 0;
    bool verbose = cmd.flags.count("v") > 0 || cmd.flags.count("verbose") > 0;
    std::string message;

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
                    message = std::format("mkdir: created directory '{}'", dir);
                    printMessage(message);
                }
            } else {
                if (fs::exists(dir)) {
                    message = std::format("mkdir: file '{}' already exists", dir);
                    printError(message);
                } else {
                    fs::create_directory(dir);
                    if (verbose) {
                        message = std::format("mkdir: created directory '{}'", dir);
                        printMessage(message);
                    }
                }
            }
        } catch (const fs::filesystem_error& e) {
            message = std::format("mkdir: failed to create directory '{}': {}", dir, e.what());
            printError(message);
        }
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
    } else if (command == "rm") {
        std::cout << "\nUsage: rm [OPTION]... [FILE]..." << std::endl;
        std::cout << "Remove (unlink) the FILE(s).\n" << std::endl;
        std::cout << "Options:" << std::endl;
        std::cout << "  -f, --force           ignore nonexistent files and arguments, never prompt" << std::endl;
        std::cout << "  -i                    prompt before every removal" << std::endl;
        std::cout << "      --interactive[=WHEN]  prompt according to WHEN: never, once (-I), or" << std::endl;
        std::cout << "                          always (-i); without WHEN, prompt always" << std::endl;
        std::cout << "  -r, -R, --recursive   remove directories and their contents recursively" << std::endl;
        std::cout << "  -v, --verbose         explain what is being done" << std::endl;
        std::cout << "      --preserve-root   do not remove '/' (default)" << std::endl;
        std::cout << "      --no-preserve-root  do not treat '/' specially" << std::endl;
        std::cout << "\nExamples:" << std::endl;
        std::cout << "  rm file.txt              Remove a file" << std::endl;
        std::cout << "  rm -i file1 file2        Remove with confirmation" << std::endl;
        std::cout << "  rm -rf directory/        Force remove directory recursively" << std::endl;
        std::cout << "  rm *.txt                 Remove all .txt files" << std::endl;
        std::cout << "  rm -i *.log              Remove all .log files with confirmation" << std::endl;

        std::cout << "\nImportant notes:" << std::endl;
        std::cout << "  - By default, rm does not remove directories." << std::endl;
        std::cout << "  - Use -r or -R to remove directories and their contents." << std::endl;
        std::cout << "  - The -f flag overrides -i and any confirmation prompts." << std::endl;
        std::cout << "  - Be cautious with 'rm -rf', it can cause data loss!" << std::endl;
    } else {
        std::cout << "No help available for: " << command << std::endl;
    }
}

void CommandHandler::printError(const std::string& message) {
    std::cerr << "Error: " << message << std::endl << std::endl;
}

void CommandHandler::printWarning(const std::string& message) {
    std::cerr << "Warning: " << message << std::endl;
}

void CommandHandler::printMessage(const std::string& message) {
    std::cout <<message << std::endl;
}

void CommandHandler::remove(const ParsedCommand &cmd) {
    if (cmd.arguments.empty()) {
        printError("rm: missing operand");
        printUsage("rm");
        return;
    }

    bool interactive = cmd.flags.count("i") > 0 || cmd.flags.count("interactive") > 0;
    bool force = cmd.flags.count("f") > 0 || cmd.flags.count("force") > 0;
    bool recursive = cmd.flags.count("r") > 0 || cmd.flags.count("recursive") > 0 || cmd.flags.count("R") > 0;
    bool verbose = cmd.flags.count("v") > 0 || cmd.flags.count("verbose") > 0;
    bool preserveRoot = cmd.flags.count("preserve-root") > 0 || cmd.flags.count("no-preserve-root") == 0;

    if (force) interactive = false;
    if (preserveRoot) {
        for (const auto& arg : cmd.arguments) {
            fs::path p(arg);
            p = fs::absolute(p);

            if (p.root_path() == p) {
                printError("rm: it is dangerous to operate recursively on '" + arg + "'");
                printError("rm: use --no-preserve-root to override this failsafe");
                return;
            }
        }
    }

    bool anyError = false;

    for (const auto& pathStr : cmd.arguments) {
        try {
            fs::path path(pathStr);

            if (!fs::exists(path)) {
                if (!force) {
                    printError("rm: cannot remove '" + pathStr + "': No such file or directory");
                    anyError = true;
                }
                continue;
            }

            if (fs::is_directory(path)) {
                if (recursive) {
                    removeDirectoryRecursive(path.string(), force, interactive);
                    if (verbose) {
                        std::cout << "removed directory '" << pathStr << "'" << std::endl;
                    }
                } else {
                    printError("rm: cannot remove '" + pathStr + "': Is a directory");
                    anyError = true;
                }
            } else {
                removeFile(path.string(), force, interactive);
                if (verbose) {
                    std::cout << "removed file '" << pathStr << "'" << std::endl;
                }
            }
        } catch (const fs::filesystem_error& e) {
            printError("rm: cannot remove '" + pathStr + "': " + e.what());
            anyError = true;
        } catch (const std::exception& e) {
            printError("rm: error processing '" + pathStr + "': " + std::string(e.what()));
            anyError = true;
        }
    }

    if (anyError && !force) {
        std::cerr << "rm: some files could not be removed" << std::endl;
    }
}

bool CommandHandler::confirmDeletion(const std::string &path, bool interactive) {
    if (!interactive) {
        return true;
    }

    std::cout << "rm: remove '" << path << "'? [y/n] ";
    std::string response;
    std::getline(std::cin, response);

    return !response.empty() && (response[0] == 'y' || response[0] == 'Y');
}

bool CommandHandler::confirmChange(const std::string &file, bool interactive) {
    if (!interactive) {
        return true;
    }

    std::cout << "touch: rewrite '" << file << "'? [y/n] ";
    std::string response;
    std::getline(std::cin, response);

    return !response.empty() && (response[0] == 'y' || response[0] == 'Y');
}

void CommandHandler::removeFile(const std::string &path, bool force, bool interactive) {
    if (!confirmDeletion(path, interactive)) {
        return;
    }

    try {
        if (!fs::remove(path)) {
            if (!force) {
                throw fs::filesystem_error("Cannot remove file", fs::path(path),std::error_code());
            }
        }
    } catch (const fs::filesystem_error& e) {
        if (!force) {
            throw;
        }
    }
}

void CommandHandler::removeDirectoryRecursive(const std::string &path, bool force, bool interactive) {
    if (fs::is_empty(path)) {
        if (confirmDeletion(path, interactive)) {
            try {
                fs::remove(path);
            } catch (...) {
                if (!force) throw;
            }
        }
        return;
    }

    if (interactive) {
        std::cout << "rm: descend into directory '" << path << "'? [y/n] ";
        std::string response;
        std::getline(std::cin, response);

        if (!(response[0] == 'y' || response[0] == 'Y') || response.empty()) {
            return;
        }
    }

    try {
        for (const auto& entry : fs::directory_iterator(path)) {
            std::string entryPath = entry.path().string();

            if (fs::is_directory(entry.status())) {
                removeDirectoryRecursive(entryPath, force, interactive);
            } else {
                removeFile(entryPath, force, interactive);
            }
        }

        if (confirmDeletion(path, interactive)) {
            fs::remove(path);
        }
    } catch (const fs::filesystem_error& e) {
        if (!force) {
            throw;
        }

        try {
            fs::remove_all(path);
        } catch (...) { }
    }
}