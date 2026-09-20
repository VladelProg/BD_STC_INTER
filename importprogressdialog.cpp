#include "importprogressdialog.h"
#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <QPushButton>

ImportProgressDialog::ImportProgressDialog(QWidget *parent) : QDialog(parent), ui(new Ui::ImportProgressDialog)
{
    ui->setupUi(this);
    setWindowTitle("Импорт данных");
    setMinimumWidth(450);
}

void ImportProgressDialog::setTotalFiles(int total) {
    ui->progress->setRange(0, total);
    ui->progress->setValue(0);
}

void ImportProgressDialog::onProgress(int processed, int total) {
    ui->progress->setMaximum(total);
    ui->progress->setValue(processed);
    ui->lblStatus->setText(QString("Обработано файлов: %1 из %2").arg(processed).arg(total));
}

void ImportProgressDialog::onSuccess(int count) {
    successCount += count;
    ui->lblSuccess->setText(QString("Успешно импортировано записей: %1").arg(successCount));
}

void ImportProgressDialog::onError(const QString &filename, const QString &msg) {
    errorCount++;
    ui->lblErrors->setText(QString("Ошибок: %1").arg(errorCount));
    ui->txtErrors->append(QString("<b>%1</b>: %2").arg(filename, msg));
}

void ImportProgressDialog::onFinished(int, int, int, const QStringList &) {
    ui->lblStatus->setText("Импорт завершён.");
    // ui->lblStatus->setText(QString("Импорт завершен. Обработано файлов: %1").arg(total));
    // ui->lblSuccess->setText(QString("Успешно импортировано записей: %1").arg(success));
    // ui->lblErrors->setText(QString("Ошибок при обработке файлов: %1").arg(errors));
}
