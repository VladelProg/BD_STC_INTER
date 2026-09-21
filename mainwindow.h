#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTableView>
#include <QPushButton>
#include <QThread>
#include "editortablemodel.h"
#include "databasemanager.h"
#include "importworker.h"
#include "importprogressdialog.h"
#include "ui_mainwindow.h"

namespace  Ui { class MainWindow; }

class MainWindow : public QWidget {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onImportClicked();
    void onClearClicked();
    void onTableContextMenu(const QPoint &pos);
    void onDeleteSelected();
    void onEditSelected();
    void onExportSelectedJson();
    void onExportSelectedCsv();

    // From workers
    void onAllItemsLoaded(const QList<EditorItem> &items);
    void onItemsInserted(const QList<EditorItem> &items);
    void onItemUpdated(const EditorItem &item);
    void onItemsDeleted(const QList<int> &ids);
    void onAllItemsCleared();
    void onFileParsed(const QString &filename, const QList<EditorItem> &items);
    void onFileError(const QString &filename, const QString &msg);
    void onImportProgress(int processed, int total);
    void onImportFinished(int total, int success, int errors, const QStringList &errFiles);
    void onDbError(const QString &msg);

signals:
    void requestLoadAll();
    void requestInsertItems(const QList<EditorItem> &items);
    void requestUpdateItem(const EditorItem &item);
    void requestDeleteItems(const QList<int> &ids);
    void requestClearAll();
    void requestStartImport(const QString &folder);

private:
    void init();
    void setupConnections();
    void exportToJson(const QList<EditorItem> &items);
    void exportToCsv(const QList<EditorItem> &items);

    Ui::MainWindow *ui;
    EditorTableModel   *model       = nullptr;

    DatabaseManager    *dbManager   = nullptr;
    QThread            *dbThread    = nullptr;
    ImportWorker       *importWorker= nullptr;
    QThread            *importThread= nullptr;

    ImportProgressDialog *progressDlg = nullptr;
};

#endif
