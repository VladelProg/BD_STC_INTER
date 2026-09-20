#ifndef IMPORTWORKER_H
#define IMPORTWORKER_H

#include <QObject>
#include <QString>
#include <QStringList>
#include "editoritem.h"

class ImportWorker : public QObject {
    Q_OBJECT
public:
    explicit ImportWorker(QObject *parent = nullptr);

public slots:
    void startImport(const QString &folderPath);

signals:
    void fileParsed(const QString &filename, const QList<EditorItem> &items);
    void fileError(const QString &filename, const QString &errorMsg);
    void progressChanged(int processedFiles, int totalFiles);
    void importFinished(int totalFiles, int successCount, int errorCount, const QStringList &errorFiles);

private:
    bool parseJsonFile(const QString &filePath, QList<EditorItem> &out, QString &error);
    bool parseCsvFile(const QString &filePath, QList<EditorItem> &out, QString &error);
    QStringList parseCsvLine(const QString &line);
};

#endif // IMPORTWORKER_H
