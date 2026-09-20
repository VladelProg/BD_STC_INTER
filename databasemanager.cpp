#include "databasemanager.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

DatabaseManager::DatabaseManager(const QString &dbPath, QObject *parent)
    : QObject(parent), dbPath(dbPath) {}

DatabaseManager::~DatabaseManager() {
    if (db.isOpen()) db.close();
}

void DatabaseManager::initDatabase() {
    QString connName = QString("db_worker_%1").arg((quintptr)QThread::currentThreadId());
    db = QSqlDatabase::addDatabase("QSQLITE", connName);
    db.setDatabaseName(dbPath);
    if (!db.open()) {
        emit errorOccurred("Не удалось открыть БД: " + db.lastError().text());
        return;
    }
    if (!ensureTable()) {
        emit errorOccurred("Не удалось создать таблицу: " + db.lastError().text());
    }
}

bool DatabaseManager::ensureTable() {
    QSqlQuery q(db);
    return q.exec(
        "CREATE TABLE IF NOT EXISTS editors ("
        " id INTEGER PRIMARY KEY AUTOINCREMENT,"
        " texteditor TEXT,"
        " fileformats TEXT,"
        " encoding TEXT,"
        " hasintellisense INTEGER,"
        " hasplugins INTEGER,"
        " cancompile INTEGER,"
        " source_file TEXT"
        ")"
        );
}

void DatabaseManager::loadAllItems() {
    QList<EditorItem> items;
    QSqlQuery q(db);
    if (!q.exec("SELECT id, texteditor, fileformats, encoding, hasintellisense, hasplugins, cancompile, source_file FROM editors")) {
        emit errorOccurred(q.lastError().text());
        return;
    }
    while (q.next()) {
        EditorItem it;
        it.id              = q.value(0).toInt();
        it.textEditor      = q.value(1).toString();
        it.fileFormats     = q.value(2).toString();
        it.encoding        = q.value(3).toString();
        it.hasIntellisense = q.value(4).toBool();
        it.hasPlugins      = q.value(5).toBool();
        it.canCompile      = q.value(6).toBool();
        it.sourceFile      = q.value(7).toString();
        items.append(it);
    }
    emit allItemsLoaded(items);
}

void DatabaseManager::insertItems(const QList<EditorItem> &items) {
    QList<EditorItem> result;
    QSqlQuery q(db);
    q.prepare(
        "INSERT INTO editors (texteditor, fileformats, encoding, hasintellisense, hasplugins, cancompile, source_file) "
        "VALUES (:te, :ff, :enc, :intel, :plug, :comp, :src)"
        );
    for (const EditorItem &it : items) {
        q.bindValue(":te",    it.textEditor);
        q.bindValue(":ff",    it.fileFormats);
        q.bindValue(":enc",   it.encoding);
        q.bindValue(":intel", it.hasIntellisense ? 1 : 0);
        q.bindValue(":plug",  it.hasPlugins ? 1 : 0);
        q.bindValue(":comp",  it.canCompile ? 1 : 0);
        q.bindValue(":src",   it.sourceFile);
        if (q.exec()) {
            EditorItem inserted = it;
            inserted.id = q.lastInsertId().toInt();
            result.append(inserted);
        }
    }
    emit itemsInserted(result);
}

void DatabaseManager::updateItem(const EditorItem &item) {
    QSqlQuery q(db);
    q.prepare(
        "UPDATE editors SET texteditor=:te, fileformats=:ff, encoding=:enc, "
        "hasintellisense=:intel, hasplugins=:plug, cancompile=:comp, source_file=:src WHERE id=:id"
        );
    q.bindValue(":te",    item.textEditor);
    q.bindValue(":ff",    item.fileFormats);
    q.bindValue(":enc",   item.encoding);
    q.bindValue(":intel", item.hasIntellisense ? 1 : 0);
    q.bindValue(":plug",  item.hasPlugins ? 1 : 0);
    q.bindValue(":comp",  item.canCompile ? 1 : 0);
    q.bindValue(":src",   item.sourceFile);
    q.bindValue(":id",    item.id);
    if (q.exec()) emit itemUpdated(item);
    else emit errorOccurred(q.lastError().text());
}

void DatabaseManager::deleteItems(const QList<int> &ids) {
    QSqlQuery q(db);
    q.prepare("DELETE FROM editors WHERE id = :id");
    QList<int> deleted;
    for (int id : ids) {
        q.bindValue(":id", id);
        if (q.exec()) deleted.append(id);
    }
    emit itemsDeleted(deleted);
}

void DatabaseManager::clearAll() {
    QSqlQuery q(db);
    if (q.exec("DELETE FROM editors"))
        emit allItemsCleared();
    else
        emit errorOccurred(q.lastError().text());
}
