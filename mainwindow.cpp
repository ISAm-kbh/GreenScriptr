#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "greenfile.h"
#include "greencommand.h"
#include "greenwindow.h"
#include <optional>
#include <QJsonObject>
#include <QTreeWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QListWidget>
#include <QList>
#include <QFileDialog>
#include <QDir>

MainWindow::MainWindow(QWidget *parent, const std::filesystem::path &_filePath)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , scriptCommand()
    , scriptWorkingDirectory()
    , scriptFilePath()
    , commandProcess(new QProcess(this))
    , fileOpenStatus(false)
    , defArgItemEdit(true)
    , fillArgItemEdit(true)
    , fillArgReorderInsert(false)
    , closeWindowAfterRun(false)
    , fillArgReorderIndices(std::tuple<int, int>(0, 0))
{
    ui->setupUi(this);
    if (!_filePath.empty()) {
        openFile(_filePath);
    }
    setupSlots();
    this->setTitle();
}

MainWindow::~MainWindow()
{
    delete ui;
    delete commandProcess;
}

void MainWindow::openFile(const std::filesystem::path &_filePath) {

    GreenFile fileHandle(_filePath);
    std::optional<QJsonObject> fileObject = fileHandle.loadJsonFromFile();

    if (!fileObject.has_value()) {
        this->ui->consoleOutputTextBox->append("> File read error.");
        return;
    }

    std::optional<QString> fileWorkingDir = GreenFile::loadWorkingDirFromJson(fileObject.value());
    std::optional<GreenCommand> fileCommand = GreenFile::loadCommandFromJson(fileObject.value()); 
    
    if (!fileWorkingDir.has_value()) {
        this->ui->consoleOutputTextBox->append("Json parse error: Working directory.");
        return;
    }

    if (!fileCommand.has_value()) {
        this->ui->consoleOutputTextBox->append("Json parse error: Command.");
        return;
    }
    
    this->scriptWorkingDirectory = std::move(fileWorkingDir.value());
    this->scriptCommand = std::move(fileCommand.value());

    this->ui->directoryLineEdit->setText(this->scriptWorkingDirectory);
    this->ui->scriptPathLineEdit->setText(this->scriptCommand.path);
    this->ui->defaultArgListWidget->addItems(this->scriptCommand.defaultArgs);

    this->ui->consoleOutputTextBox->append("> File loaded.");

    for (std::tuple<QString, QString> arg : this->scriptCommand.fillableArgs) {
        QString left = std::get<0>(arg);
        QString right = std::get<1>(arg);

        QTreeWidgetItem *argItem = new QTreeWidgetItem(this->ui->fillableArgTreeWidget);
        argItem->setText(0, left);
        argItem->setText(1, right);
    }

    fileOpenStatus = true;
    scriptFilePath = _filePath;
    this->ui->actionSave->setEnabled(true);
    this->setTitle();
}

bool MainWindow::saveFile() {
    if (!fileOpenStatus) {
        this->ui->consoleOutputTextBox->append("> Save error: No file loaded.");
        return false;
    }

    GreenFile fileHandle(scriptFilePath);
    QJsonObject objectWorkingDir = GreenFile::encodeWorkingDirIntoJson(scriptWorkingDirectory);
    QJsonObject objectCommand = GreenFile::encodeCommandIntoJson(scriptCommand);

    if (!fileHandle.encodeJsonToFile(objectWorkingDir, objectCommand)) {
        this->ui->consoleOutputTextBox->append("> Save error: Couldn't write to file.");
        return false;
    }

    this->ui->consoleOutputTextBox->append("> Saved.");
    return true;
}

bool MainWindow::createAndSaveFile(const std::filesystem::path &_filePath) {
    GreenFile fileHandle(_filePath);
    QJsonObject objectWorkingDir = GreenFile::encodeWorkingDirIntoJson(scriptWorkingDirectory);
    QJsonObject objectCommand = GreenFile::encodeCommandIntoJson(scriptCommand);

    if (!fileHandle.encodeJsonToFile(objectWorkingDir, objectCommand)) {
        this->ui->consoleOutputTextBox->append("> Save error: Couldn't write file.");
        return false;
    }

    this->fileOpenStatus = true;
    this->scriptFilePath = _filePath;
    this->setTitle();
    QString consoleMessage = QString::fromStdString(_filePath.string());
    consoleMessage.prepend("> Saved as \"");
    consoleMessage.append("\".");
    this->ui->consoleOutputTextBox->append(consoleMessage);

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

void MainWindow::setTitle() {
    if (!fileOpenStatus) {
        this->setWindowTitle("Untitled - GreenScriptr");
        return;
    }

    QString openFileTitle = QString::fromStdString(this->scriptFilePath.stem().string());
    openFileTitle.append(" - GreenScriptr");
    this->setWindowTitle(openFileTitle);
}

void MainWindow::runCommand() {
    this->ui->runButton->setEnabled(false);
    this->ui->actionNew->setEnabled(false);
    this->ui->actionOpen->setEnabled(false);
    this->ui->actionClose_Window->setEnabled(false);
    this->ui->actionRun->setEnabled(false);
    this->ui->actionRun_and_Exit->setEnabled(false);

    this->ui->actionSave->setEnabled(false);
    this->ui->actionSave_As->setEnabled(false);

    this->propogateAllBinds();
    this->commandProcess->setProgram(this->scriptCommand.path);
    this->commandProcess->setWorkingDirectory(this->scriptWorkingDirectory);
    this->commandProcess->setArguments(this->scriptCommand.AssembleArguments());

    this->ui->actionSave->setEnabled(true);
    this->ui->actionSave_As->setEnabled(true);
    
    QString commandLineCommand = this->scriptCommand.AssembleCommand();
    commandLineCommand.prepend("> ");
    this->ui->consoleOutputTextBox->append(commandLineCommand);
    this->commandProcess->start();
}


void MainWindow::setupSlots() {
    QObject::connect(this->ui->directoryLineEdit, &QLineEdit::editingFinished,
            this, &MainWindow::directoryEdited);
    QObject::connect(this->ui->fileSelectButton, &QPushButton::clicked,
            this, &MainWindow::directoryFileDialog);
    QObject::connect(this->ui->scriptPathLineEdit, &QLineEdit::editingFinished,
            this, &MainWindow::pathEdited);

    QObject::connect(this->ui->defaultArgAddButton, &QPushButton::clicked,
            this, &MainWindow::defArgAdded);
    QObject::connect(this->ui->defaultArgListWidget->model(), &QAbstractItemModel::rowsMoved,
            this, &MainWindow::defArgsReordered);
    QObject::connect(this->ui->defaultArgListWidget, &QListWidget::itemSelectionChanged,
            this, &MainWindow::defArgSelected);
    QObject::connect(this->ui->defaultArgListWidget, &QListWidget::itemChanged,
            this, &MainWindow::defArgEdited);
    QObject::connect(this->ui->defaultArgDeleteButton, &QPushButton::clicked,
            this, &MainWindow::defArgDeleted);

    QObject::connect(this->ui->fillableArgAddButton, &QPushButton::clicked,
            this, &MainWindow::fillArgAdded);
    QObject::connect(this->ui->fillableArgTreeWidget, &QTreeWidget::itemChanged,
            this, &MainWindow::fillArgEdited);
    QObject::connect(this->ui->fillableArgTreeWidget->model(),
            &QAbstractItemModel::rowsAboutToBeRemoved,
            this, &MainWindow::fillArgsPrereorder);
    QObject::connect(this->ui->fillableArgTreeWidget->model(),
            &QAbstractItemModel::rowsInserted,
            this, &MainWindow::fillArgsReordered);
    QObject::connect(this->ui->fillableArgTreeWidget, &QTreeWidget::itemSelectionChanged,
            this, &MainWindow::fillArgSelected);
    QObject::connect(this->ui->fillableArgDeleteButton, &QPushButton::clicked,
            this, &MainWindow::fillArgDeleted);

    QObject::connect(this->ui->runButton, &QPushButton::clicked,
            this, &MainWindow::runButtonClicked);
    QObject::connect(this->commandProcess, &QProcess::errorOccurred,
            this, &MainWindow::commandError);
    QObject::connect(this->commandProcess, &QProcess::channelReadyRead,
            this, &MainWindow::commandOutputReady);
    QObject::connect(this->commandProcess, &QProcess::finished,
            this, &MainWindow::commandProcessDone);

    QObject::connect(this->ui->actionNew, &QAction::triggered,
            this, &MainWindow::fileActionNew);
    QObject::connect(this->ui->actionNew_Window, &QAction::triggered,
            this, &MainWindow::fileActionNewWindow);
    QObject::connect(this->ui->actionOpen, &QAction::triggered,
            this, &MainWindow::fileActionOpen);
    QObject::connect(this->ui->actionSave, &QAction::triggered,
            this, &MainWindow::fileActionSave);
    QObject::connect(this->ui->actionSave_As, &QAction::triggered,
            this, &MainWindow::fileActionSaveAs);
    QObject::connect(this->ui->actionClose_Window, &QAction::triggered,
            this, &MainWindow::fileActionCloseWindow);

    QObject::connect(this->ui->actionRun, &QAction::triggered,
            this, &MainWindow::runActionRun);
    QObject::connect(this->ui->actionRun_and_Exit, &QAction::triggered,
            this, &MainWindow::runActionRunAndExit);
}

void MainWindow::directoryEdited() {
    this->scriptWorkingDirectory = this->ui->directoryLineEdit->text();
}

void MainWindow::directoryFileDialog() {
    QFileDialog dialog(this);
    dialog.setFileMode(QFileDialog::Directory);
    dialog.setViewMode(QFileDialog::List);
    dialog.setOptions(QFileDialog::ShowDirsOnly);
    dialog.setAcceptMode(QFileDialog::AcceptOpen);

    QDir startingDir = QDir::home();
    if (!this->scriptWorkingDirectory.isEmpty()) {
        QDir storedDir = QDir(this->scriptWorkingDirectory);
        if (storedDir.exists()) {
            startingDir = storedDir;
        }
    }
    dialog.setDirectory(startingDir);
    
    if (dialog.exec()) {
        QStringList resultList = dialog.selectedFiles();
        if (resultList.count() == 0) {
            return;
        }
        this->ui->directoryLineEdit->setText(resultList.first());
        this->scriptWorkingDirectory = resultList.first();
    }
}

void MainWindow::pathEdited() {
    this->scriptCommand.path = this->ui->scriptPathLineEdit->text();
}

void MainWindow::defArgAdded() {
    this->ui->defaultArgListWidget->addItem("new argument");
    this->scriptCommand.defaultArgs.append("new argument");

    int endIndex = this->ui->defaultArgListWidget->count() - 1;
    QListWidgetItem *newItem = this->ui->defaultArgListWidget->item(endIndex);
    this->defArgItemEdit = false;
    newItem->setFlags(newItem->flags() | Qt::ItemIsEditable);

    this->ui->defaultArgListWidget->editItem(newItem);
}

void MainWindow::defArgsReordered(const QModelIndex &sourceParent, int sourceStart,
            int sourceEnd, const QModelIndex &destinationParent, int destinationRow) {
    QStringList subList;
    for (int i = sourceStart; i <= sourceEnd; i++) {
        subList.append(this->scriptCommand.defaultArgs[i]);
    }

    this->scriptCommand.defaultArgs.remove(sourceStart, subList.count());
    int adjustedDestinationRow = destinationRow;
    if (destinationRow > sourceStart) {
        adjustedDestinationRow -= subList.count();
    }

    for (int i = 0; i < subList.count(); i++) {
        this->scriptCommand.defaultArgs.insert((adjustedDestinationRow + i), subList[i]);
    }
}

void MainWindow::defArgSelected() {
    if (!this->ui->defaultArgDeleteButton->isEnabled()) {
        this->ui->defaultArgDeleteButton->setEnabled(true);
    } 
}

void MainWindow::defArgEdited(QListWidgetItem *itemChanged) {
    if (!this->defArgItemEdit) {
        this->defArgItemEdit = true;
        return;
    }

    QString newText = itemChanged->text();
    int itemRow = this->ui->defaultArgListWidget->row(itemChanged);

    this->scriptCommand.defaultArgs[itemRow] = newText;
}

void MainWindow::defArgDeleted() {
    QList<QListWidgetItem *> selectedItemList = this->ui->defaultArgListWidget->selectedItems();
    if (selectedItemList.count() != 1) {
        return;
    }

    QListWidgetItem *itemToDelete = selectedItemList.first();
    int itemRow = this->ui->defaultArgListWidget->row(itemToDelete);
    int itemIndex = this->scriptCommand.defaultArgs.indexOf(itemToDelete->text());

    this->scriptCommand.defaultArgs.removeAt(itemIndex);
    this->ui->defaultArgListWidget->takeItem(itemRow);
    delete itemToDelete;

    if (this->ui->defaultArgListWidget->count() == 0) {
        this->ui->defaultArgDeleteButton->setEnabled(false);
    }
}

void MainWindow::fillArgAdded() {
    QTreeWidgetItem *newItem = new QTreeWidgetItem(this->ui->fillableArgTreeWidget);
    this->fillArgItemEdit = false;
    newItem->setText(0, "");
    this->fillArgItemEdit = false;
    newItem->setText(1, "");

    this->scriptCommand.fillableArgs.append(std::tuple<QString, QString>("", ""));
    
    this->fillArgItemEdit = false;
    newItem->setFlags((newItem->flags() | Qt::ItemIsEditable) & ~Qt::ItemIsDropEnabled);
    this->ui->fillableArgTreeWidget->editItem(newItem, 0);
}

void MainWindow::fillArgEdited(QTreeWidgetItem *itemChanged, int column) {
    if (!this->fillArgItemEdit) {
        this->fillArgItemEdit = true;
        return;
    }
    if (column < 0) {
        this->fillArgItemEdit = true;
        return;
    }
    
    QString newText = itemChanged->text(column);
    int itemRow = this->ui->fillableArgTreeWidget->indexOfTopLevelItem(itemChanged);
    switch (column) {
        case 0:
            std::get<0>(this->scriptCommand.fillableArgs[itemRow]) = newText;
            break;
        case 1:
            std::get<1>(this->scriptCommand.fillableArgs[itemRow]) = newText;
            break;
    } 
}

void MainWindow::fillArgsPrereorder(const QModelIndex &parent, int first, int last) {
    this->fillArgReorderInsert = true;
    this->fillArgReorderIndices = std::tuple<int, int>(first, last);
}

void MainWindow::fillArgsReordered(const QModelIndex &parent, int first, int last) {
    if (!this->fillArgReorderInsert) {
        return;
    }

    QList<std::tuple<QString, QString>> subList;
    int subStart, subEnd;
    std::tie(subStart, subEnd) = this->fillArgReorderIndices;
    for (int i = subStart; i <= subEnd; i++) {
        subList.append(this->scriptCommand.fillableArgs[i]);
    }

    this->scriptCommand.fillableArgs.remove(subStart, subList.count());

    for (int i = 0; i < subList.count(); i++) {
        this->scriptCommand.fillableArgs.insert((first + i), subList[i]);
    }

    this->fillArgReorderInsert = false;
}

void MainWindow::fillArgSelected() {
    if (!this->ui->fillableArgDeleteButton->isEnabled()) {
        this->ui->fillableArgDeleteButton->setEnabled(true);
    }
}

void MainWindow::fillArgDeleted() {
    QList<QTreeWidgetItem *> selectedItemList = this->ui->fillableArgTreeWidget->selectedItems();
    if (selectedItemList.count() != 1) {
        return;
    }

    QTreeWidgetItem *itemToDelete = selectedItemList.first();
    int widgetIndex = this->ui->fillableArgTreeWidget->indexOfTopLevelItem(itemToDelete);

    std::tuple<QString, QString> itemAsTuple(itemToDelete->text(0), itemToDelete->text(1));
    int itemIndex = this->scriptCommand.fillableArgs.indexOf(itemAsTuple);

    this->scriptCommand.fillableArgs.removeAt(itemIndex);
    this->ui->fillableArgTreeWidget->takeTopLevelItem(widgetIndex);
    delete itemToDelete;

    if (this->ui->fillableArgTreeWidget->topLevelItemCount() == 0) {
        this->ui->fillableArgDeleteButton->setEnabled(false);
    }
}

void MainWindow::runButtonClicked() {
    this->runCommand();
}

void MainWindow::commandError(QProcess::ProcessError error) {
    if (error == QProcess::FailedToStart) {
        this->ui->consoleOutputTextBox->append(
                "> Program failed to start, verify the path & permissions.");
        this->ui->runButton->setEnabled(true);
        this->ui->actionNew->setEnabled(true);
        this->ui->actionOpen->setEnabled(true);
        this->ui->actionClose_Window->setEnabled(true);
        this->ui->actionRun->setEnabled(true);
        this->ui->actionRun_and_Exit->setEnabled(true);

        if (this->closeWindowAfterRun) {
            this->close();
        }
    }
}

void MainWindow::commandOutputReady(int channel) {
    this->commandProcess->setCurrentReadChannel(channel);
    while (this->commandProcess->canReadLine()) {
        QString line = QString::fromLocal8Bit(this->commandProcess->readLine());
        this->ui->consoleOutputTextBox->append(line);
    }
}

void MainWindow::commandProcessDone(int exitCode) {
    QString exitCodeMessage(QString::number(exitCode));
    exitCodeMessage.prepend("> exited with code: ");
    exitCodeMessage.append(".");
    this->ui->consoleOutputTextBox->append(exitCodeMessage);

    this->ui->runButton->setEnabled(true);
    this->ui->actionNew->setEnabled(true);
    this->ui->actionOpen->setEnabled(true);
    this->ui->actionClose_Window->setEnabled(true);
    this->ui->actionRun->setEnabled(true);
    this->ui->actionRun_and_Exit->setEnabled(true);

    if (this->closeWindowAfterRun) {
        this->close();
    }
}

void MainWindow::fileActionNew() {
    this->scriptCommand.clear();
    this->scriptWorkingDirectory.clear();

    this->ui->directoryLineEdit->clear();
    this->ui->scriptPathLineEdit->clear();
    this->ui->defaultArgListWidget->clear();
    this->ui->fillableArgTreeWidget->clear();

    this->ui->defaultArgDeleteButton->setEnabled(false);
    this->ui->fillableArgDeleteButton->setEnabled(false);
    
    this->scriptFilePath.clear();
    this->fileOpenStatus = false;

    this->defArgItemEdit = true;
    this->fillArgItemEdit = true;
    this->fillArgReorderInsert = false;
    this->fillArgReorderIndices = std::tuple<int, int>(0, 0);

    this->ui->actionSave->setEnabled(true);
    this->setTitle();
    this->ui->consoleOutputTextBox->append("> New File.");
}

void MainWindow::fileActionNewWindow() const {
    MainWindow *newWin = new MainWindow();
    GreenWindow::windows.append(newWin);
    newWin->show();
}

void MainWindow::fileActionOpen() {
    QFileDialog dialog(this);
    dialog.setFileMode(QFileDialog::ExistingFile);
    dialog.setViewMode(QFileDialog::List);
    dialog.setAcceptMode(QFileDialog::AcceptOpen);

    dialog.setDirectory(QDir::home());
    if (dialog.exec()) {
        QStringList resultList = dialog.selectedFiles();
        if (resultList.count() == 0) {
            return;
        }
        QString resultString = resultList.first();
        std::filesystem::path resultPath = std::filesystem::path(resultString.toStdString());
        this->openFile(resultPath);
    }
}

void MainWindow::fileActionSave() {
    if (!this->fileOpenStatus) {
        return;
    }
    this->saveFile();
}

void MainWindow::fileActionSaveAs() {
    QFileDialog dialog(this);
    dialog.setFileMode(QFileDialog::ExistingFile);
    dialog.setViewMode(QFileDialog::List);
    dialog.setAcceptMode(QFileDialog::AcceptSave);

    if (fileOpenStatus) {
        QDir parentDir = QDir(this->scriptFilePath.parent_path());
        dialog.setDirectory(parentDir);
    } else {
        dialog.setDirectory(QDir::home());
    }

    if (dialog.exec()) {
        QStringList resultList = dialog.selectedFiles();
        if (resultList.count() == 0) {
            return;
        }
        QString resultString = resultList.first();
        std::filesystem::path resultPath = std::filesystem::path(resultString.toStdString());
        this->createAndSaveFile(resultPath);
    }
}

void MainWindow::fileActionCloseWindow() {
    this->close();
}

void MainWindow::runActionRun() {
    this->runCommand();
}

void MainWindow::runActionRunAndExit() {
    this->closeWindowAfterRun = true;
    this->runCommand();
}
