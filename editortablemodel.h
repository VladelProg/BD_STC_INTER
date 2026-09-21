#ifndef EDITORTABLEMODEL_H
#define EDITORTABLEMODEL_H

#include <QAbstractTableModel>
#include <QList>
#include "editoritem.h"

class EditorTableModel : public QAbstractTableModel {
    Q_OBJECT
public:
    enum /*Column*/class Column : int {
        TextEditor = 0, FileFormats, Encoding,
        HasIntellisense, HasPlugins, CanCompile
    };

    explicit EditorTableModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;

    const QList<EditorItem>& items() const { return _items; }
    EditorItem itemAt(int row) const;
    int  idAtRow(int row) const;

    void setItems(const QList<EditorItem> &items);
    void appendItems(const QList<EditorItem> &items);
    void updateItem(const EditorItem &item);
    void removeRowsByIds(const QList<int> &ids);
    void clear();

private:
    QList<EditorItem> _items;
    static constexpr uint8_t colCount = 6;
};

#endif // EDITORTABLEMODEL_H
