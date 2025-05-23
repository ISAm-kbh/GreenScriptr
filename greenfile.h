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
        inline static const QString jsonKeyWorkingDir = "workingDirectory";
        inline static const QString jsonKeyScriptPath = "scriptPath";
        inline static const QString jsonKeyDefaultArgs = "defaultArguments";
        inline static const QString jsonKeyFillableArgs = "fillableArguments";

        GreenFile(const std::filesystem::path &_filePath) : file(_filePath) {}

        std::optional<QJsonObject> loadJsonFromFile();
        static std::optional<GreenCommand> loadCommandFromJson(const QJsonObject &json);
        static std::optional<QString> loadWorkingDirFromJson(const QJsonObject &json);

        static QJsonObject encodeWorkingDirIntoJson(const QString &_workingDir);
        static QJsonObject encodeCommandIntoJson(const GreenCommand &command);
        bool encodeJsonToFile(const QJsonObject &_workingDirectory, const QJsonObject &_command);
};

#endif // GREENFILE_H
