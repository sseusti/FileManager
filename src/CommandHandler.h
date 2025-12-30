#ifndef COMMANDHANDLER_H
#define COMMANDHANDLER_H

#include <map>
#include <string>
#include <functional>
#include <vector>
#include <filesystem>

#include "CommandParser.h"

namespace fs = std::filesystem;

class CommandHandler {
public:
    CommandHandler();

    void parseAndExecute(const std::string& input);
    void executeParsed(const ParsedCommand& cmd);

    void registerCommand(const std::string& name, std::function<void(const ParsedCommand&)> handler);

private:
    std::map<std::string, std::function<void(const ParsedCommand&)>> commands;

    static void printWorkingDirectory();
    static void listDirectory();
    static void changeDirectory(const ParsedCommand& cmd);
    static void makeDirectory(const ParsedCommand& cmd);
    static void remove(const ParsedCommand& cmd);
    static void showHelp(const ParsedCommand& cmd);

    static void printError(const std::string& message);

    static std::vector<std::string> tokenize(const std::string& input);
    static void printUsage(const std::string& command);
};

#endif