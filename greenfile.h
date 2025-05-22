#ifndef GREENFILE_H
#define GREENFILE_H

#include "greencommand.h"
#include <optional>
#include <QJsonObject>
#include <QFile>
#include <filesystem>

class GreenFile
{
    public:
        QFile file;

        GreenFile(const std::filesystem::path &_filePath) : file(_filePath) {}

        bool openFile();
        std::optional<QJsonObject> loadJsonFromFile();
        std::optional<GreenCommand> loadCommandFromJson(const QJsonObject &json) const;

};

#endif // GREENFILE_H
