#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QObject>
#include <QList>
#include <QSqlDatabase>
#include "editoritem.h"
#include <QThread>

class DatabaseManager : public QObject {
    Q_OBJECT
public:
    explicit DatabaseManager(const QString &dbPath, QObject *parent = nullptr);
    ~DatabaseManager();

public slots:
    void initDatabase();
    void loadAllItems();
    void insertItems(const QList<EditorItem> &items);
    void updateItem(const EditorItem &item);
    void deleteItems(const QList<int> &ids);
    void clearAll();

signals:
    void allItemsLoaded(const QList<EditorItem> &items);
    void itemsInserted(const QList<EditorItem> &itemsWithIds);
    void itemUpdated(const EditorItem &item);
    void itemsDeleted(const QList<int> &ids);
    void allItemsCleared();
    void errorOccurred(const QString &msg);

private:
    QString dbPath;
    QSqlDatabase db;
    bool ensureTable();
};

#endif // DATABASEMANAGER_H
