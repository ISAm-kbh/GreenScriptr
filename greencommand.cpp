#include "greencommand.h"

using std::string;
using std::vector;
using std::tuple;

GreenCommand::GreenCommand() {
    path = string();
    defaultArgs = vector<string>();
    fillableArgs = vector<tuple<string, string>>();
}
