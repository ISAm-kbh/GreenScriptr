#ifndef GREENFILE_H
#define GREENFILE_H

#include "greencommand.h"
#include <QString>
#include <optional>

class GreenFile
{
public:
    GreenFile() = delete;
    static std::optional<GreenCommand> loadFromFile(const QString &filePath);
};

#endif // GREENFILE_H
