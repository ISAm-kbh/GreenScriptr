#include "greenfile.h"
#include <optional>
#include <QJsonValue>
#include <QJsonArray>
#include <QString>
#include <QStringList>
#include <QList>
#include <tuple>

using std::tuple;

std::optional<GreenCommand> GreenFile::loadCommandFromJson(const QJsonObject &json) const {
    const QJsonValue valuePath = json["path"];
    if (!valuePath.isString()) {
        return std::nullopt;
    }
    const QString stringPath = valuePath.toString();

    const QJsonValue valueDefArgs = json["defaultArguments"];
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

    const QJsonValue valueFillArgs = json["fillableArguments"];
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
