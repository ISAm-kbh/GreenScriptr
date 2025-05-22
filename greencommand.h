#ifndef GREENCOMMAND_H
#define GREENCOMMAND_H

#include <string>
#include <vector>
#include <tuple>

class GreenCommand
{
    public:
        std::string path;
        std::vector<std::string> defaultArgs;
        std::vector< std::tuple< std::string, std::string >> fillableArgs;

        GreenCommand();
        GreenCommand(const GreenCommand &);
        GreenCommand(GreenCommand &&);
        GreenCommand &operator=(const GreenCommand &);
        GreenCommand &operator=(GreenCommand &&);

        GreenCommand(
            const std::string &_path, 
            const std::vector<std::string> &_defArgs,
            const std::vector<std::tuple<std::string, std::string>> &_fillArgs)
            : path(_path), defaultArgs(_defArgs), fillableArgs(_fillArgs) {}

        std::string AssembleCommand() const;
};

#endif // GREENCOMMAND_H
