#include "greenfile.h"
#include <QFile>

std::optional<GreenCommand> GreenFile::loadFromFile(const QString &filePath) {
    QFile file(filePath);

    if (!file.open(QIODevice::ReadWrite | QIODevice::Text)) {
        return std::nullopt;
    }

    return GreenCommand();
}
