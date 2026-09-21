#ifndef EDITDIALOG_H
#define EDITDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QCheckBox>
#include "editoritem.h"
#include "ui_editdialog.h"

namespace Ui { class EditDialog; }


class EditDialog : public QDialog {
    Q_OBJECT
public:
    explicit EditDialog(QWidget *parent = nullptr);
    ~EditDialog();
    void setItem(const EditorItem &item);
    EditorItem item() const;

private:
    Ui::EditDialog *ui;
    int id         = -1;
    QString source;
};

#endif
