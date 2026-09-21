#ifndef EDITORITEM_H
#define EDITORITEM_H

#include <QString>
#include <QStringList>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>

struct EditorItem {
    int     id              = -1;
    QString textEditor;
    QString fileFormats;
    QString encoding;
    bool    hasIntellisense = false;
    bool    hasPlugins      = false;
    bool    canCompile      = false;
    QString sourceFile;

    QJsonObject toJson() const;
    static EditorItem fromJsonObject(const QJsonObject &obj, const QString &source = QString());
    static EditorItem fromCsvRow(const QStringList &row, const QStringList &headers, const QString &source = QString());
    static bool toBool(const QString &s);
};

Q_DECLARE_METATYPE(EditorItem)
Q_DECLARE_METATYPE(QList<EditorItem>)

#endif // EDITORITEM_H
