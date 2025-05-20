#include "greencommand.h"
#include <regex>

using std::string;
using std::vector;
using std::tuple;

GreenCommand::GreenCommand() {
    path = string();
    defaultArgs = vector<string>();
    fillableArgs = vector<tuple<string, string>>();
}

string GreenCommand::AssembleCommand() const {
    string finalCommand = "";

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
