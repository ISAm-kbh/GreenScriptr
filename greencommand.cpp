#include "greencommand.h"
#include <regex>

using std::string;
using std::vector;
using std::tuple;

GreenCommand::GreenCommand() {
    this->path = string();
    this->defaultArgs = vector<string>();
    this->fillableArgs = vector<tuple<string, string>>();
}

GreenCommand::GreenCommand(const GreenCommand & other) {
    this->path = other.path;
    this->defaultArgs = other.defaultArgs;
    this->fillableArgs = other.fillableArgs;
}

GreenCommand::GreenCommand(GreenCommand && other) {
    this->path = std::move(other.path);
    this->defaultArgs = std::move(other.defaultArgs);
    this->fillableArgs = std::move(other.fillableArgs);
}


GreenCommand::GreenCommand(string _path,
        vector<string> _defArgs,
        vector<tuple<string, string>> _fillArgs) {
    this->path = string(_path);
    this->defaultArgs = vector<string>(_defArgs);
    this->fillableArgs = vector<tuple<string, string>>(_fillArgs);
}

string GreenCommand::AssembleCommand() const {
    string finalCommand = string();

    finalCommand.append(path);
    
    std::regex whitespace("^\\s*$");
    for (const string & arg : defaultArgs) {
        if (arg.empty()) {
            continue;
        }
        if (std::regex_match(arg, whitespace)) {
            continue;
        }
        
        finalCommand.append(" ");
        finalCommand.append(arg);
    }

    for (const tuple<string, string> & arg : fillableArgs) {
        string left = std::get<0>(arg);
        bool leftEmpty = left.empty() || std::regex_match(left, whitespace);
        string right = std::get<1>(arg);
        bool rightEmpty = right.empty() || std::regex_match(right, whitespace);

        if (!leftEmpty) {
            finalCommand.append(" ");
            finalCommand.append(left);
        }

        if (!rightEmpty) {
            finalCommand.append(" ");
            finalCommand.append(right);
        }
    }

    return finalCommand;
}

GreenCommand& GreenCommand::operator=(const GreenCommand & other) {
    if (this == &other) {
       return *this;
    } 

    this->path = other.path;
    this->defaultArgs = other.defaultArgs;
    this->fillableArgs = other.fillableArgs;

    return *this;
}

GreenCommand& GreenCommand::operator=(GreenCommand && other) {
    if (this == &other) {
        return *this;
    }

    this->path = std::move(other.path);
    this->defaultArgs = std::move(other.defaultArgs);
    this->fillableArgs = std::move(other.fillableArgs);
    
    return *this;
}
