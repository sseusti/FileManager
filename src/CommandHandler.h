#ifndef COMMANDHANDLER_H
#define COMMANDHANDLER_H

#include <map>
#include <string>
#include <functional>
#include <vector>
#include <filesystem>

namespace fs = std::filesystem;

class CommandHandler {
public:
    CommandHandler();

    void parseCommands(const std::string& input);
    void executeCommands(const std::string& cmd, const std::vector<std::string>& args);

private:
    std::map<std::string, std::function<void(const std::vector<std::string>&)>> commands;

    static std::vector<std::string> parseInput(const std::string& input);

    static void printWorkingDirectory();
    static void listDirectory();
    static void changeDirectory(const std::vector<std::string>& args);
    static void makeDirectory(const std::vector<std::string>& args);
    static void remove(const std::vector<std::string>& args);

    static bool validateArguments(const std::vector<std::string>& args, size_t expected, const std::string& command);
    static void printError(const std::string& message);
};
#endif