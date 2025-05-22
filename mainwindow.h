#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QString>
#include <filesystem>
#include "greencommand.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void openFile(const std::filesystem::path &_filePath);

private:
    Ui::MainWindow *ui;
    QString scriptWorkingDirectory;
    GreenCommand scriptCommand;
};
#endif // MAINWINDOW_H
