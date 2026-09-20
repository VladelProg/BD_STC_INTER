#include "editoritem.h"

bool EditorItem::toBool(const QString &s) {
    QString t = s.trimmed().toLower();
    return (t == "true" || t == "1" || t == "yes");
}

// Вспомогательная функция: разбивает строку по ";" и тримит части
static QJsonValue toJsonArrayOrString(const QString &s) {
    QStringList parts = s.split(';', Qt::SkipEmptyParts);
    for (QString &p : parts)
        p = p.trimmed();

    // Убираем возможные пустые строки после трима
    parts.removeAll(QString());

    if (parts.size() <= 1) {
        return QJsonValue(s.trimmed());
    }

    QJsonArray arr;
    for (const QString &p : parts)
        arr.append(p);
    return arr;  // Несколько значений — массив
}

QJsonObject EditorItem::toJson() const {
    QJsonObject obj;
    obj["texteditor"]      = textEditor;
    obj["fileformats"]     = toJsonArrayOrString(fileFormats);
    obj["encoding"]        = toJsonArrayOrString(encoding);
    obj["hasintellisense"] = hasIntellisense;
    obj["hasplugins"]      = hasPlugins;
    obj["cancompile"]      = canCompile;
    return obj;
}

// QJsonObject EditorItem::toJson() const {
//     QJsonObject obj;
//     obj["texteditor"]      = textEditor;
//     obj["fileformats"]     = fileFormats;
//     obj["encoding"]        = encoding;
//     obj["hasintellisense"] = hasIntellisense;
//     obj["hasplugins"]      = hasPlugins;
//     obj["cancompile"]      = canCompile;
//     return obj;
// }

static QString trimKey(const QString &k) {
    return k.trimmed().toLower();
}

static QString extractString(const QJsonValue &v) {
    if (v.isString()) return v.toString().trimmed();
    if (v.isArray()) {
        QStringList parts;
        for (const QJsonValue &av : v.toArray())
            parts << av.toString().trimmed();
        return parts.join("; ");
    }
    return v.toVariant().toString().trimmed();
}

EditorItem EditorItem::fromJsonObject(const QJsonObject &obj, const QString &source) {
    EditorItem item;
    item.sourceFile = source;

    // Если есть вложенный объект (например "root"), берём его
    QJsonObject data = obj;
    if (obj.size() == 1) {
        QJsonValue firstVal = obj.begin().value();
        if (firstVal.isObject())
            data = firstVal.toObject();
    }

    for (auto it = data.begin(); it != data.end(); ++it) {
        QString key = trimKey(it.key());
        const QJsonValue &val = it.value();
        if      (key == "texteditor")      item.textEditor      = extractString(val);
        else if (key == "fileformats")     item.fileFormats     = extractString(val);
        else if (key == "encoding")        item.encoding        = extractString(val);
        else if (key == "hasintellisense") item.hasIntellisense = toBool(extractString(val));
        else if (key == "hasplugins")      item.hasPlugins      = toBool(extractString(val));
        else if (key == "cancompile")      item.canCompile      = toBool(extractString(val));
    }
    return item;
}

EditorItem EditorItem::fromCsvRow(const QStringList &row, const QStringList &headers, const QString &source) {
    EditorItem item;
    item.sourceFile = source;
    for (int i = 0; i < qMin(row.size(), headers.size()); ++i) {
        QString key = headers[i].trimmed().toLower();
        QString val = row[i].trimmed();
        if      (key == "texteditor")      item.textEditor      = val;
        else if (key == "fileformats")     item.fileFormats     = val;
        else if (key == "encoding")        item.encoding        = val;
        else if (key == "hasintellisense") item.hasIntellisense = toBool(val);
        else if (key == "hasplugins")      item.hasPlugins      = toBool(val);
        else if (key == "cancompile")      item.canCompile      = toBool(val);
    }
    return item;
}
