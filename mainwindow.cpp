#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "greenfile.h"
#include "greencommand.h"
#include <optional>
#include <QJsonObject>
#include <QTreeWidgetItem>
#include <tuple>
#include <QLineEdit>
#include <QPushButton>
#include <QListWidget>
#include <QList>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , scriptCommand()
    , scriptWorkingDirectory()
    , scriptFilePath()
    , fileOpenStatus(false)
    , defArgItemEdit(true)
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
}

void MainWindow::directoryEdited() {
    this->scriptWorkingDirectory = this->ui->directoryLineEdit->text();
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
    for (int i = 0; i < subList.count(); i++) {
        this->scriptCommand.defaultArgs.insert((destinationRow + i), subList[i]);
    }
}

void MainWindow::defArgSelected() {
    if (!this->ui->defaultArgDeleteButton->isEnabled()) {
        this->ui->defaultArgDeleteButton->setEnabled(true);
    } 
}

void MainWindow::defArgEdited(QListWidgetItem *itemChanged) {
    if (!defArgItemEdit) {
        defArgItemEdit = true;
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

    this->ui->defaultArgDeleteButton->setEnabled(false);
}
