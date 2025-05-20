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
            std::string _path, std::string _defArgs,
            std::vector<std::tuple<std::string, std::string>> _fillableArgs);

        std::string AssembleCommand() const;
};

#endif // GREENCOMMAND_H
