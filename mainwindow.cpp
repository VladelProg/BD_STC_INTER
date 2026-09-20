#include "mainwindow.h"
#include "editdialog.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QMessageBox>
#include <QMenu>
#include <QAction>
#include <QHeaderView>
#include <QApplication>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QFile>
#include <QTextStream>

MainWindow::MainWindow(QWidget *parent) : QWidget(parent), ui(new Ui::MainWindow) {

    ui->setupUi(this);
    init();

    QString dbPath = QApplication::applicationDirPath() + "/editors.db";
    dbManager = new DatabaseManager(dbPath);
    dbThread  = new QThread(this);
    dbManager->moveToThread(dbThread);

    connect(dbThread, &QThread::started, dbManager, &DatabaseManager::initDatabase);
    connect(dbManager, &DatabaseManager::initDatabase, this, &MainWindow::requestLoadAll, Qt::QueuedConnection);
    connect(dbManager, &DatabaseManager::initDatabase, dbManager, &DatabaseManager::loadAllItems, Qt::QueuedConnection);

    connect(dbManager, &DatabaseManager::allItemsLoaded, this, &MainWindow::onAllItemsLoaded);
    connect(dbManager, &DatabaseManager::itemsInserted,  this, &MainWindow::onItemsInserted);
    connect(dbManager, &DatabaseManager::itemUpdated,    this, &MainWindow::onItemUpdated);
    connect(dbManager, &DatabaseManager::itemsDeleted,   this, &MainWindow::onItemsDeleted);
    connect(dbManager, &DatabaseManager::allItemsCleared,this, &MainWindow::onAllItemsCleared);
    connect(dbManager, &DatabaseManager::errorOccurred,  this, &MainWindow::onDbError);

    connect(this, &MainWindow::requestLoadAll,     dbManager, &DatabaseManager::loadAllItems);
    connect(this, &MainWindow::requestInsertItems, dbManager, &DatabaseManager::insertItems);
    connect(this, &MainWindow::requestUpdateItem,  dbManager, &DatabaseManager::updateItem);
    connect(this, &MainWindow::requestDeleteItems, dbManager, &DatabaseManager::deleteItems);
    connect(this, &MainWindow::requestClearAll,    dbManager, &DatabaseManager::clearAll);

    dbThread->start();

    importWorker = new ImportWorker();
    importThread = new QThread(this);
    importWorker->moveToThread(importThread);

    connect(this, &MainWindow::requestStartImport, importWorker, &ImportWorker::startImport);
    connect(importWorker, &ImportWorker::fileParsed,      this, &MainWindow::onFileParsed);
    connect(importWorker, &ImportWorker::fileError,       this, &MainWindow::onFileError);
    connect(importWorker, &ImportWorker::progressChanged, this, &MainWindow::onImportProgress);
    connect(importWorker, &ImportWorker::importFinished,  this, &MainWindow::onImportFinished);

    importThread->start();
}

MainWindow::~MainWindow() {
    disconnect(ui->tableView, &QTableView::customContextMenuRequested, this, &MainWindow::onTableContextMenu);

    disconnect(ui->btnImport, &QPushButton::clicked, this, &MainWindow::onImportClicked);
    disconnect(ui->btnClear,  &QPushButton::clicked, this, &MainWindow::onClearClicked);

    importThread->quit();
    importThread->wait();
    dbThread->quit();
    dbThread->wait();

    delete ui;
}

void MainWindow::init() {
    setWindowTitle("Editor");
    resize(900, 500);

    model = new EditorTableModel(this);

    ui->tableView->setModel(model);
    ui->tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableView->setSelectionMode(QAbstractItemView::ExtendedSelection);
    ui->tableView->horizontalHeader()->setStretchLastSection(true);
    ui->tableView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->tableView, &QTableView::customContextMenuRequested, this, &MainWindow::onTableContextMenu);

    connect(ui->btnImport, &QPushButton::clicked, this, &MainWindow::onImportClicked);
    connect(ui->btnClear,  &QPushButton::clicked, this, &MainWindow::onClearClicked);
}

void MainWindow::onImportClicked() {
    QString folder = QFileDialog::getExistingDirectory(this, "Выберите папку с JSON/CSV файлами");
    if (folder.isEmpty()) return;

    progressDlg = new ImportProgressDialog(this);
    progressDlg->show();

    emit requestStartImport(folder);
}

void MainWindow::onClearClicked() {
    if (QMessageBox::question(this, "Подтверждение", "Очистить все записи?") == QMessageBox::Yes)
        emit requestClearAll();
}

void MainWindow::onTableContextMenu(const QPoint &pos) {
    QMenu menu(this);
    QAction *actEdit   = menu.addAction("Редактировать");
    QAction *actDelete = menu.addAction("Удалить");
    menu.addSeparator();
    QAction *actExpJson = menu.addAction("Экспорт в JSON");
    QAction *actExpCsv  = menu.addAction("Экспорт в CSV");

    bool hasSelection = ui->tableView->selectionModel()->hasSelection();
    actEdit->setEnabled(hasSelection);
    actDelete->setEnabled(hasSelection);
    actExpJson->setEnabled(hasSelection);
    actExpCsv->setEnabled(hasSelection);

    QAction *chosen = menu.exec(ui->tableView->viewport()->mapToGlobal(pos));
    if (!chosen) return;
    if      (chosen == actEdit)   onEditSelected();
    else if (chosen == actDelete) onDeleteSelected();
    else if (chosen == actExpJson) onExportSelectedJson();
    else if (chosen == actExpCsv)  onExportSelectedCsv();
}

void MainWindow::onDeleteSelected() {
    QList<int> ids;
    for (const QModelIndex &idx : ui->tableView->selectionModel()->selectedRows())
        ids << model->idAtRow(idx.row());
    if (!ids.isEmpty()) emit requestDeleteItems(ids);
}

void MainWindow::onEditSelected() {
    QModelIndexList sel = ui->tableView->selectionModel()->selectedRows();
    if (sel.isEmpty()) return;
    EditorItem it = model->itemAt(sel.first().row());
    EditDialog dlg(this);
    dlg.setItem(it);
    if (dlg.exec() == QDialog::Accepted)
        emit requestUpdateItem(dlg.item());
}

void MainWindow::onExportSelectedJson() {
    QList<EditorItem> items;
    for (const QModelIndex &idx : ui->tableView->selectionModel()->selectedRows())
        items << model->itemAt(idx.row());
    exportToJson(items);
}

void MainWindow::onExportSelectedCsv() {
    QList<EditorItem> items;
    for (const QModelIndex &idx : ui->tableView->selectionModel()->selectedRows())
        items << model->itemAt(idx.row());
    exportToCsv(items);
}

void MainWindow::exportToJson(const QList<EditorItem> &items) {
    if (items.isEmpty()) return;

    QString path = QFileDialog::getSaveFileName(this, "Экспорт в JSON", QString(), "JSON (*.json)");
    if (path.isEmpty()) return;

    QFileInfo fi(path);
    if (fi.suffix().isEmpty()) {
        path += ".json";
    }

    QFile f(path);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Text)) return;

    if (items.size() == 1) {
        QJsonObject root;
        root["root"] = items.first().toJson();
        f.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
    } else {
        QJsonArray arr;
        for (const EditorItem &it : items) {
            QJsonObject root;
            root["root"] = it.toJson();
            arr.append(root);
        }
        f.write(QJsonDocument(arr).toJson(QJsonDocument::Indented));
    }
}

void MainWindow::exportToCsv(const QList<EditorItem> &items) {
    QString path = QFileDialog::getSaveFileName(this, "Экспорт в CSV", QString(), "CSV (*.csv)");
    if (path.isEmpty()) return;

    QFileInfo fi(path);
    if (fi.suffix().isEmpty()) {
        path += ".csv";
    }

    QFile f(path);

    if (!f.open(QIODevice::WriteOnly | QIODevice::Text)) return;
    QTextStream out(&f);
    out.setCodec("UTF-8");
    out << "texteditor,fileformats,encoding,hasintellisense,hasplugins,cancompile\n";
    for (const EditorItem &it : items) {
        auto q = [](const QString &s) {
            return "\"" + QString(s).replace("\"","\"\"") + "\"";
        };
        out << q(it.textEditor) << "," << q(it.fileFormats) << "," << q(it.encoding) << ","
            << (it.hasIntellisense ? "true" : "false") << ","
            << (it.hasPlugins      ? "true" : "false") << ","
            << (it.canCompile      ? "true" : "false") << "\n";
    }
}

void MainWindow::onAllItemsLoaded(const QList<EditorItem> &items) { model->setItems(items); }
void MainWindow::onItemsInserted(const QList<EditorItem> &items)  { model->appendItems(items); }
void MainWindow::onItemUpdated(const EditorItem &item)            { model->updateItem(item); }
void MainWindow::onItemsDeleted(const QList<int> &ids)            { model->removeRowsByIds(ids); }
void MainWindow::onAllItemsCleared()                              { model->clear(); }

void MainWindow::onFileParsed(const QString &filename, const QList<EditorItem> &items) {
    Q_UNUSED(filename)
    emit requestInsertItems(items);
    if (progressDlg) progressDlg->onSuccess(items.size());
}

void MainWindow::onFileError(const QString &filename, const QString &msg) {
    if (progressDlg) progressDlg->onError(filename, msg);
}

void MainWindow::onImportProgress(int processed, int total) {
    if (progressDlg) progressDlg->onProgress(processed, total);
}

void MainWindow::onImportFinished(int total, int success, int errors, const QStringList &errFiles) {
    if (progressDlg) progressDlg->onFinished(total, success, errors, errFiles);
}

void MainWindow::onDbError(const QString &msg) {
    QMessageBox::warning(this, "Ошибка БД", msg);
}
