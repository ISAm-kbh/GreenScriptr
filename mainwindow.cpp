#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "greenfile.h"
#include "greencommand.h"
#include <optional>
#include <QJsonObject>
#include <QTreeWidgetItem>
#include <tuple>
#include <QLineEdit>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , scriptCommand()
    , scriptWorkingDirectory()
    , scriptFilePath()
    , fileOpenStatus(false)
{
    ui->setupUi(this);
    setupSlots();
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

// Safety function to quickly ensure all UI items are stored into their container classes
// Use before:
// - Saving
// - Running
void MainWindow::propogateAllBinds() {
    this->scriptWorkingDirectory = this->ui->directoryLineEdit->text();
    this->scriptCommand.path = this->ui->scriptPathLineEdit->text();
    
    for(int i = 0; i < this->ui->defaultArgListWidget->count(); i++) {
        QString itemText = this->ui->defaultArgListWidget->item(i)->text();
        if (i == this->scriptCommand.defaultArgs.count()) {
            this->scriptCommand.defaultArgs.append(itemText);
            continue;
        } 
        if (this->scriptCommand.defaultArgs[i] == itemText) {
            continue;
        }

        this->scriptCommand.defaultArgs[i] = itemText;
    }

    for(int i = 0; i < this->ui->fillableArgTreeWidget->topLevelItemCount(); i++) {
        QTreeWidgetItem *item = this->ui->fillableArgTreeWidget->topLevelItem(i);
        std::tuple<QString, QString> itemTuple(item->text(0), item->text(1));
        
        if (i == this->scriptCommand.fillableArgs.count()) {
            this->scriptCommand.fillableArgs.append(itemTuple);
            continue;
        }
        if (this->scriptCommand.fillableArgs[i] == itemTuple) {
            continue;
        }


        this->scriptCommand.fillableArgs[i] = itemTuple; 
    }
}

void MainWindow::setupSlots() {
    QObject::connect(this->ui->directoryLineEdit, &QLineEdit::editingFinished,
            this, &MainWindow::directoryEdited);
    QObject::connect(this->ui->scriptPathLineEdit, &QLineEdit::editingFinished,
            this, &MainWindow::pathEdited);
}

void MainWindow::directoryEdited() {
    this->scriptWorkingDirectory = this->ui->directoryLineEdit->text();
}

void MainWindow::pathEdited() {
    this->scriptCommand.path = this->ui->scriptPathLineEdit->text();
}
