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

    void registerCommand(const std::string& name, const std::function<void(const ParsedCommand&)>& handler);

private:
    std::map<std::string, std::function<void(const ParsedCommand&)>> commands;

    static void printWorkingDirectory();
    static void listDirectory(const ParsedCommand& cmd);
    static void changeDirectory(const ParsedCommand& cmd);
    static void makeDirectory(const ParsedCommand& cmd);
    static void remove(const ParsedCommand& cmd);
    static void showHelp(const ParsedCommand& cmd);
    static void touch(const ParsedCommand& cmd);

    static bool confirmDeletion(const std::string& path, bool interactive);
    static bool isHidden(const auto& entry);
    static bool confirmChange(const std::string& path, bool interactive);
    static void removeFile(const std::string& path, bool force, bool interactive);
    static void removeDirectoryRecursive(const std::string& path, bool force, bool interactive);

    static void printError(const std::string& message);
    static void printWarning(const std::string& message);
    static void printMessage(const std::string& message);

    static std::vector<std::string> tokenize(const std::string& input);
    static void printUsage(const std::string& command);
};

#endif