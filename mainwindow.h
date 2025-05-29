#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QString>
#include <filesystem>
#include "greencommand.h"
#include <QModelIndex>
#include <QListWidgetItem>
#include <QTreeWidgetItem>
#include <tuple>
#include <QProcess>

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
    bool saveFile();
    void propogateAllBinds();
private:
    Ui::MainWindow *ui;
    QString scriptWorkingDirectory;
    GreenCommand scriptCommand;
    bool fileOpenStatus;
    std::filesystem::path scriptFilePath;
    QProcess *commandProcess;

    bool defArgItemEdit;
    bool fillArgItemEdit;
    bool fillArgReorderInsert;

    std::tuple<int, int> fillArgReorderIndices;

    void setTitle();
    void runCommand();
    void setupSlots();
private slots:
    void directoryEdited();
    void directoryFileDialog();
    void pathEdited();

    void defArgAdded();
    void defArgsReordered(const QModelIndex &sourceParent, int sourceStart,
            int sourceEnd, const QModelIndex &destinationParent, int destinationRow);
    void defArgSelected();
    void defArgEdited(QListWidgetItem *itemChanged);
    void defArgDeleted();

    void fillArgAdded();
    void fillArgEdited(QTreeWidgetItem *itemChanged, int column);
    void fillArgsPrereorder(const QModelIndex &parent, int first, int last);
    void fillArgsReordered(const QModelIndex &parent, int first, int last);
    void fillArgSelected();
    void fillArgDeleted();

    void runButtonClicked();
    void commandError(QProcess::ProcessError error);
    void commandOutputReady(int channel);
    void commandProcessDone(int exitCode);
};
#endif // MAINWINDOW_H
