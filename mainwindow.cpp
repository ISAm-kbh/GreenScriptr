#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "greenfile.h"
#include "greencommand.h"
#include <optional>
#include <QJsonObject>
#include <QTreeWidgetItem>
#include <tuple>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , scriptCommand()
    , scriptWorkingDirectory()
    , scriptFilePath()
    , fileOpenStatus(false)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::openFile(const std::filesystem::path &_filePath) {

    GreenFile fileHandle(_filePath);
    std::optional<QJsonObject> fileObject = fileHandle.loadJsonFromFile();

    if (!fileObject.has_value()) {
        this->ui->consoleOutputTextBox->setText("File read error");
        return;
    }

    std::optional<QString> fileWorkingDir = GreenFile::loadWorkingDirFromJson(fileObject.value());
    std::optional<GreenCommand> fileCommand = GreenFile::loadCommandFromJson(fileObject.value()); 
    
    if (!fileWorkingDir.has_value()) {
        this->ui->consoleOutputTextBox->setText("Json parse error: Working directory");
        return;
    }

    if (!fileCommand.has_value()) {
        this->ui->consoleOutputTextBox->setText("Json parse error: Command");
        return;
    }
    
    this->scriptWorkingDirectory = std::move(fileWorkingDir.value());
    this->scriptCommand = std::move(fileCommand.value());

    this->ui->directoryLineEdit->setText(this->scriptWorkingDirectory);
    this->ui->scriptPathLineEdit->setText(this->scriptCommand.path);
    this->ui->defaultArgListWidget->addItems(this->scriptCommand.defaultArgs);

    this->ui->consoleOutputTextBox->setText("");

    for (std::tuple<QString, QString> arg : this->scriptCommand.fillableArgs) {
        QString left = std::get<0>(arg);
        QString right = std::get<1>(arg);

        QTreeWidgetItem *argItem = new QTreeWidgetItem(this->ui->fillableArgTreeWidget);
        argItem->setText(0, left);
        argItem->setText(1, right);
    }

    fileOpenStatus = true;
    scriptFilePath = _filePath;
}

bool MainWindow::saveFile() {
    if (!fileOpenStatus) {
        this->ui->consoleOutputTextBox->setText("Save error: No file loaded");
        return false;
    }

    GreenFile fileHandle(scriptFilePath);
    QJsonObject objectWorkingDir = GreenFile::encodeWorkingDirIntoJson(scriptWorkingDirectory);
    QJsonObject objectCommand = GreenFile::encodeCommandIntoJson(scriptCommand);

    if (!fileHandle.encodeJsonToFile(objectWorkingDir, objectCommand)) {
        this->ui->consoleOutputTextBox->setText("Save error: Couldn't write to file");
        return false;
    }

    this->ui->consoleOutputTextBox->setText("Saved.");
    return true;
}
