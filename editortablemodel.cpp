#include "editortablemodel.h"

//const uint8_t EditorTableModel::colCount = 6;

EditorTableModel::EditorTableModel(QObject *parent) : QAbstractTableModel(parent) {}

int EditorTableModel::rowCount(const QModelIndex &) const { return _items.size(); }
int EditorTableModel::columnCount(const QModelIndex &) const { return colCount; }

QVariant EditorTableModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid() || index.row() >= _items.size()) return {};
    const EditorItem &it = _items[index.row()];
    if (role == Qt::DisplayRole || role == Qt::EditRole) {
        switch (static_cast<Column>(index.column())) {
        case Column::TextEditor:      return it.textEditor;
        case Column::FileFormats:     return it.fileFormats;
        case Column::Encoding:        return it.encoding;
        case Column::HasIntellisense: return it.hasIntellisense ? "Да" : "Нет";
        case Column::HasPlugins:      return it.hasPlugins      ? "Да" : "Нет";
        case Column::CanCompile:      return it.canCompile      ? "Да" : "Нет";
        }
    }
    if (role == Qt::TextAlignmentRole) return int(Qt::AlignCenter);
    return {};
}

QVariant EditorTableModel::headerData(int section, Qt::Orientation ori, int role) const {
    if (ori != Qt::Horizontal || role != Qt::DisplayRole) return {};
    switch (static_cast<Column>(section)) {
    case Column::TextEditor:      return "Редактор";
    case Column::FileFormats:     return "Форматы";
    case Column::Encoding:        return "Кодировка";
    case Column::HasIntellisense: return "IntelliSense";
    case Column::HasPlugins:      return "Плагины";
    case Column::CanCompile:      return "Компиляция";
    }
    return {};
}

Qt::ItemFlags EditorTableModel::flags(const QModelIndex &) const {
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

EditorItem EditorTableModel::itemAt(int row) const {
    if (row >= 0 && row < _items.size()) return _items[row];
    return {};
}

int EditorTableModel::idAtRow(int row) const {
    if (row >= 0 && row < _items.size()) return _items[row].id;
    return -1;
}

void EditorTableModel::setItems(const QList<EditorItem> &items) {
    beginResetModel();
    _items = items;
    endResetModel();
}

void EditorTableModel::appendItems(const QList<EditorItem> &items) {
    if (items.isEmpty()) return;
    beginInsertRows(QModelIndex(), _items.size(), _items.size() + items.size() - 1);
    _items.append(items);
    endInsertRows();
}

void EditorTableModel::updateItem(const EditorItem &item) {
    for (int i = 0; i < _items.size(); ++i) {
        if (_items[i].id == item.id) {
            _items[i] = item;
            emit dataChanged(index(i, 0), index(i, colCount - 1));
            return;
        }
    }
}

void EditorTableModel::removeRowsByIds(const QList<int> &ids) {
    for (int id : ids) {
        for (int i = 0; i < _items.size(); ++i) {
            if (_items[i].id == id) {
                beginRemoveRows(QModelIndex(), i, i);
                _items.removeAt(i);
                endRemoveRows();
                break;
            }
        }
    }
}

void EditorTableModel::clear() {
    beginResetModel();
    _items.clear();
    endResetModel();
}
