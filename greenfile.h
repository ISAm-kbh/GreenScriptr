#ifndef GREENFILE_H
#define GREENFILE_H

#include "greencommand.h"
#include <optional>
#include <QJsonObject>
#include <Qfile>

class GreenFile
{
    public:
        GreenFile();

        std::optional<GreenCommand> loadCommandFromJson(const QJsonObject &json) const;

};

#endif // GREENFILE_H
