#include "greenfile.h"
#include <QIODevice>
#include <QByteArray>
#include <QJsonDocument>
#include <QJsonValue>
#include <QJsonArray>
#include <QStringList>
#include <QList>
#include <QTextStream>
#include <tuple>

using std::tuple;

std::optional<QJsonObject> GreenFile::loadJsonFromFile() {
    if (this->file.isOpen()) {
        this->file.close();
    }

    if (!this->file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return std::nullopt;
    }
    
    QByteArray fileContents = this->file.readAll();
    this->file.close();

    QJsonParseError parseError;
    QJsonDocument fileContentsParsed = QJsonDocument::fromJson(fileContents, &parseError);
    if (fileContentsParsed.isNull()) {
        return std::nullopt;
    }

    if (!fileContentsParsed.isObject()) {
        return std::nullopt;
    }

    return fileContentsParsed.object();
}

std::optional<GreenCommand> GreenFile::loadCommandFromJson(const QJsonObject &json) const {
    const QJsonValue valuePath = json[this->jsonKeyScriptPath];
    if (!valuePath.isString()) {
        return std::nullopt;
    }
    const QString stringPath = valuePath.toString();

    const QJsonValue valueDefArgs = json[this->jsonKeyDefaultArgs];
    if (!valueDefArgs.isArray()) {
        return std::nullopt;
    }
    const QJsonArray arrayDefArgs = valueDefArgs.toArray();

    QStringList stringListDefArgs;
    for (const QJsonValue arrayValue : arrayDefArgs) {
        if (!arrayValue.isString()) {
            return std::nullopt;
        }

        QString stringArrayValue = arrayValue.toString();
        stringListDefArgs.append(stringArrayValue);
    }

    const QJsonValue valueFillArgs = json[this->jsonKeyFillableArgs];
    if (!valueFillArgs.isArray()) {
        return std::nullopt;
    }
    const QJsonArray arrayFillArgs = valueFillArgs.toArray();
    
    QList<tuple<QString, QString>> tupleListFillArgs;
    for (const QJsonValue arrayValue : arrayFillArgs) {
        if (!arrayValue.isArray()) {
            return std::nullopt;
        }

        const QJsonArray subArray = arrayValue.toArray();
        if (subArray.count() != 2) {
            return std::nullopt;
        }
        
        for (const QJsonValue subArrayValue : subArray) {
            if (!subArrayValue.isString()) {
                return std::nullopt;
            }
        }

        tuple<QString, QString> arrayTuple(subArray[0].toString(), subArray[1].toString());
        tupleListFillArgs.append(arrayTuple);
    }

    return GreenCommand(stringPath, stringListDefArgs, tupleListFillArgs);
}

std::optional<QString> GreenFile::loadWorkingDirFromJson(const QJsonObject &json) const {
    const QJsonValue valueDir = json[this->jsonKeyWorkingDir];
    if (!valueDir.isString()) {
        return std::nullopt;
    }

    return valueDir.toString();
}

QJsonObject GreenFile::encodeWorkingDirIntoJson(const QString &_workingDir) const {
    QJsonObject returnObject;
    returnObject[this->jsonKeyWorkingDir] = _workingDir;
    return returnObject;
}

QJsonObject GreenFile::encodeCommandIntoJson(const GreenCommand &command) const {
    QJsonObject returnObject;

    returnObject[this->jsonKeyScriptPath] = command.path;

    QJsonArray arrayDefArgs = QJsonArray::fromStringList(command.defaultArgs);
    returnObject[this->jsonKeyDefaultArgs] = arrayDefArgs;

    QJsonArray arrayFillArgs;
    for (tuple<QString, QString> arg : command.fillableArgs) {
        QJsonValue left(std::get<0>(arg));
        QJsonValue right(std::get<1>(arg));
        QJsonArray argArray = { left, right };

        QJsonValue valueSubArray(argArray);
        arrayFillArgs.append(valueSubArray);
    }
    returnObject[this->jsonKeyFillableArgs] = arrayFillArgs;

    return returnObject;
}

bool GreenFile::encodeJsonToFile(const QJsonObject &_workingDirectory, const QJsonObject &_command) {
    if (this->file.isOpen()) {
        this->file.close();
    }

    if (!this->file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
        return false;
    }

    QVariantMap map = _workingDirectory.toVariantMap();

    map.insert(_command.toVariantMap());

    QJsonDocument document(QJsonObject::fromVariantMap(map));

    QByteArray fileContents = document.toJson(QJsonDocument::Indented);
    QTextStream out(&this->file);

    out << fileContents;
    this->file.close();

    return true;
}
