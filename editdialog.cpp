#include "editdialog.h"
#include <QFormLayout>
#include <QDialogButtonBox>
#include <QVBoxLayout>

EditDialog::EditDialog(QWidget *parent) : QDialog(parent), ui(new Ui::EditDialog) {
    ui->setupUi(this);
    setWindowTitle("Редактирование записи");

    connect(ui->btnEdit, &QPushButton::pressed, this, &QDialog::accept);
    //connect(ui->btnCancel,  &QPushButton::pressed, this, &EditDialog::close);
}

EditDialog::~EditDialog(){
    disconnect(ui->btnEdit, &QPushButton::pressed, this, &QDialog::accept);
    delete ui;
}

void EditDialog::setItem(const EditorItem &item) {
    id     = item.id;
    source = item.sourceFile;
    ui->edEditor->setText(item.textEditor);
    ui->edFormats->setText(item.fileFormats);
    ui->edEncoding->setText(item.encoding);
    ui->cbIntel->setChecked(item.hasIntellisense);
    ui->cbPlugins->setChecked(item.hasPlugins);
    ui->cbCompile->setChecked(item.canCompile);
}

EditorItem EditDialog::item() const {
    EditorItem it;
    it.id              = id;
    it.sourceFile      = source;
    it.textEditor      = ui->edEditor->text();
    it.fileFormats     = ui->edFormats->text();
    it.encoding        = ui->edEncoding->text();
    it.hasIntellisense = ui->cbIntel->isChecked();
    it.hasPlugins      = ui->cbPlugins->isChecked();
    it.canCompile      = ui->cbCompile->isChecked();
    return it;
}
