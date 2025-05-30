#include "greencommand.h"
#include <QRegularExpression>

using std::tuple;

GreenCommand::GreenCommand(const GreenCommand & other) {
    this->path = other.path;
    this->defaultArgs = other.defaultArgs;
    this->fillableArgs = other.fillableArgs;
}

GreenCommand::GreenCommand(GreenCommand && other) {
    this->path = std::move(other.path);
    this->defaultArgs = std::move(other.defaultArgs);
    this->fillableArgs = std::move(other.fillableArgs);
}

QString GreenCommand::AssembleCommand() const {
    QString finalCommand = QString();

    finalCommand.append(path);
    
    QRegularExpression whitespace("^\\s*$");
    for (QStringView arg : defaultArgs) {
        if (arg.empty()) {
            continue;
        }
        if (whitespace.matchView(arg).hasMatch()) {
            continue;
        }
        
        finalCommand.append(" ");
        finalCommand.append(arg);
    }

    for (const tuple<QStringView, QStringView> & arg : fillableArgs) {
        QStringView left = std::get<0>(arg);
        bool leftEmpty = left.empty() || whitespace.matchView(left).hasMatch();
        QStringView right = std::get<1>(arg);
        bool rightEmpty = right.empty() || whitespace.matchView(right).hasMatch();

        if (!leftEmpty) {
            finalCommand.append(" ");
            finalCommand.append(left);
        }

        if (!rightEmpty) {
            finalCommand.append(" ");
            finalCommand.append(right);
        }
    }

    return finalCommand;
}

QStringList GreenCommand::AssembleArguments() const {
    QStringList finalArgList;
    
    QRegularExpression whitespace("^\\s*$");
    for (QStringView arg : this->defaultArgs) {
        if (arg.empty()) {
            continue;
        }
        if (whitespace.matchView(arg).hasMatch()) {
            continue;
        }
        
        finalArgList.append(arg.toString());
    }

    for (const tuple<QStringView, QStringView> & arg : fillableArgs) {
        QString left = std::get<0>(arg).toString();
        bool leftEmpty = left.isEmpty() || whitespace.matchView(left).hasMatch();
        QString right = std::get<1>(arg).toString();
        bool rightEmpty = right.isEmpty() || whitespace.matchView(right).hasMatch();

        if (!leftEmpty) finalArgList.append(left);
        if (!rightEmpty) finalArgList.append(right);
    }

    return finalArgList;
}

GreenCommand& GreenCommand::operator=(const GreenCommand & other) {
    if (this == &other) {
       return *this;
    } 

    this->path = other.path;
    this->defaultArgs = other.defaultArgs;
    this->fillableArgs = other.fillableArgs;

    return *this;
}

GreenCommand& GreenCommand::operator=(GreenCommand && other) {
    if (this == &other) {
        return *this;
    }

    this->path = std::move(other.path);
    this->defaultArgs = std::move(other.defaultArgs);
    this->fillableArgs = std::move(other.fillableArgs);
    
    return *this;
}

void GreenCommand::clear() {
    this->path.clear();
    this->defaultArgs.clear();
    this->fillableArgs.clear();
}
