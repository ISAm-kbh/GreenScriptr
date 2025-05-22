#ifndef GREENFILE_H
#define GREENFILE_H

#include "greencommand.h"
#include <optional>
#include <QJsonObject>
#include <QFile>
#include <filesystem>
#include <QString>

class GreenFile
{
    public:
        QFile file;
        const QString jsonKeyWorkingDir = "workingDirectory";
        const QString jsonKeyScriptPath = "scriptPath";
        const QString jsonKeyDefaultArgs = "defaultArguments";
        const QString jsonKeyFillableArgs = "fillableArguments";

        GreenFile(const std::filesystem::path &_filePath) : file(_filePath) {}

        bool openFile();

        std::optional<QJsonObject> loadJsonFromFile();
        std::optional<GreenCommand> loadCommandFromJson(const QJsonObject &json) const;
        std::optional<QString> loadWorkingDirFromJson(const QJsonObject &json) const;

        QJsonObject encodeWorkingDirIntoJson(const QString &_workingDir) const;
        QJsonObject encodeCommandIntoJson(const GreenCommand &command) const;
};

#endif // GREENFILE_H
