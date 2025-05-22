#ifndef GREENFILE_H
#define GREENFILE_H

#include "greencommand.h"
#include <optional>
#include <QJsonObject>
#include <Qfile>

class GreenFile
{
    private:
        QFile file;
    public:
        GreenFile();
        GreenFile(const QString &_filePath) : file(_filePath) {}

        std::optional<GreenCommand> loadCommandFromJson(const QJsonObject &json) const;

};

#endif // GREENFILE_H
