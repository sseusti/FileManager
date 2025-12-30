#ifndef COMMANDPARSER_H
#define COMMANDPARSER_H

#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <optional>

struct ParsedCommand {
    std::string command;
    std::unordered_map<std::string, std::string> options;
    std::unordered_set<std::string> flags;
    std::vector<std::string> arguments;
    std::vector<std::string> errors;
};

class CommandParser {
public:
    static ParsedCommand parse(const std::vector<std::string>& tokens);

private:
    static bool isOption(const std::string& token);
    static bool isFlag(const std::string& token);
};

#endif