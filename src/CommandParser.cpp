#include "CommandParser.h"

bool CommandParser::isOption(const std::string &token) {
    return token.size() > 1 && token[0] == '-';
}

bool CommandParser::isFlag(const std::string &token) {
    return token.size() == 2 && token[0] == '-' && std::isalpha(token[1]);
}

ParsedCommand CommandParser::parse(const std::vector<std::string> &tokens) {
    ParsedCommand result;
    if (tokens.empty()) {
        result.errors.emplace_back("No command provided");
        return result;
    }

    result.command = tokens[0];

    for (size_t i = 1; i < tokens.size(); ++i) {
        const std::string &token = tokens[i];

        if (isOption(token)) {
            if (isFlag(token)) {
                result.flags.insert(token.substr(1));

            } else if (token.substr(0, 2) == "--") {
                size_t eq_pos = token.find('=');
                if (eq_pos != std::string::npos) {
                    std::string key = token.substr(2, eq_pos - 2);
                    std::string value = token.substr(eq_pos + 1);
                    result.options[key] = value;

                } else {
                    result.flags.insert(token.substr(2));

                }
            } else {
                std::string key = token.substr(1);
                if (i + 1 < tokens.size() && !isOption(tokens[i + 1])) {
                    result.options[key] = tokens[++i];

                } else {
                    result.flags.insert(key);

                }
            }
        } else {
            result.arguments.push_back(token);
        }
    }

    return result;
}
