#include "mainwindow.h"
#include "greenwindow.h"

#include <QApplication>
#include <QCommandLineParser>
#include <QLocale>
#include <QTranslator>
#include <QStringList>
#include <filesystem>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QCommandLineParser parser;
    parser.addPositionalArgument("file", QApplication::translate("main", "File to open"));
    parser.process(app);

    const QStringList args = parser.positionalArguments();
    QString inFile;
    if (args.size() == 1) {
        inFile = args.first();
    }
    const std::filesystem::path inFilePath(inFile.toStdString());

    QTranslator translator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString &locale : uiLanguages) {
        const QString baseName = "GreenScriptr_" + QLocale(locale).name();
        if (translator.load(":/i18n/" + baseName)) {
            app.installTranslator(&translator);
            break;
        }
    }

    MainWindow *w = new MainWindow(nullptr, inFilePath);
    GreenWindow::windows.append(w);
    w->show();
    return app.exec();
}
