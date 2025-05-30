#ifndef GREENCOMMAND_H
#define GREENCOMMAND_H

#include <QString>
#include <QStringList>
#include <QList>
#include <tuple>

class GreenCommand
{
    public:
        QString path;
        QStringList defaultArgs;
        QList<std::tuple<QString, QString>> fillableArgs;

        GreenCommand() : path(), defaultArgs(), fillableArgs() {}
        GreenCommand(const GreenCommand &);
        GreenCommand(GreenCommand &&);
        GreenCommand &operator=(const GreenCommand &);
        GreenCommand &operator=(GreenCommand &&);

        GreenCommand(
            QStringView _path,
            const QStringList &_defArgs,
            const QList<std::tuple<QString, QString>> _fillArgs)
            : path(_path), defaultArgs(_defArgs), fillableArgs(_fillArgs) {}

        QString AssembleCommand() const;
        QStringList AssembleArguments() const;

        void clear();
};

#endif // GREENCOMMAND_H
