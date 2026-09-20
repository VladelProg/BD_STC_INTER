#include "importworker.h"
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

ImportWorker::ImportWorker(QObject *parent) : QObject(parent) {}

void ImportWorker::startImport(const QString &folderPath) {
    QDir dir(folderPath);
    if (!dir.exists()) {
        emit importFinished(0, 0, 1, QStringList() << folderPath);
        return;
    }

    QStringList filters;
    filters << "*.json" << "*.csv";
    QFileInfoList files = dir.entryInfoList(filters, QDir::Files, QDir::Name);

    int totalFiles   = files.size();
    int successCount = 0;
    int errorCount   = 0;
    QStringList errorFiles;
    int processed = 0;

    for (const QFileInfo &fi : files) {
        QString ext = fi.suffix().toLower();
        QList<EditorItem> items;
        QString error;
        bool ok = false;

        if (ext == "json") ok = parseJsonFile(fi.absoluteFilePath(), items, error);
        else if (ext == "csv") ok = parseCsvFile(fi.absoluteFilePath(), items, error);
        else { error = "Неизвестный формат"; ok = false; }

        if (ok && !items.isEmpty()) {
            emit fileParsed(fi.fileName(), items);
            successCount++;
        } else {
            if (error.isEmpty()) error = "Файл не содержит данных";
            emit fileError(fi.fileName(), error);
            errorFiles << fi.fileName();
            errorCount++;
        }
        processed++;
        emit progressChanged(processed, totalFiles);
    }

    emit importFinished(totalFiles, successCount, errorCount, errorFiles);
}

bool ImportWorker::parseJsonFile(const QString &filePath, QList<EditorItem> &out, QString &error) {
    QFile f(filePath);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) {
        error = "Не удалось открыть файл";
        return false;
    }
    QJsonParseError parseErr;
    QJsonDocument doc = QJsonDocument::fromJson(f.readAll(), &parseErr);
    if (parseErr.error != QJsonParseError::NoError) {
        error = "Ошибка парсинга JSON: " + parseErr.errorString();
        return false;
    }
    if (!doc.isObject()) { error = "JSON не является объектом"; return false; }

    EditorItem item = EditorItem::fromJsonObject(doc.object(), QFileInfo(filePath).fileName());
    out.append(item);
    return true;
}

bool ImportWorker::parseCsvFile(const QString &filePath, QList<EditorItem> &out, QString &error) {
    QFile f(filePath);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) {
        error = "Не удалось открыть файл";
        return false;
    }
    QTextStream in(&f);
    in.setCodec("UTF-8");

    if (in.atEnd()) { error = "Пустой файл"; return false; }

    QString headerLine = in.readLine();
    QStringList headers = parseCsvLine(headerLine);
    if (headers.isEmpty()) { error = "Нет заголовков"; return false; }

    QString srcName = QFileInfo(filePath).fileName();
    while (!in.atEnd()) {
        QString line = in.readLine();
        if (line.trimmed().isEmpty()) continue;
        QStringList row = parseCsvLine(line);
        out.append(EditorItem::fromCsvRow(row, headers, srcName));
    }
    return !out.isEmpty();
}

QStringList ImportWorker::parseCsvLine(const QString &line) {
    QStringList result;
    QString current;
    bool inQuotes = false;
    for (int i = 0; i < line.size(); ++i) {
        QChar c = line[i];
        if (inQuotes) {
            if (c == '"') {
                if (i + 1 < line.size() && line[i+1] == '"') {
                    current += '"'; ++i;
                } else {
                    inQuotes = false;
                }
            } else {
                current += c;
            }
        } else {
            if (c == '"') inQuotes = true;
            else if (c == ',') { result << current; current.clear(); }
            else current += c;
        }
    }
    result << current;
    return result;
}
