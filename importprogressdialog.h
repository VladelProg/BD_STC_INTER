#ifndef IMPORTPROGRESSDIALOG_H
#define IMPORTPROGRESSDIALOG_H

#include <QDialog>
#include <QProgressBar>
#include <QLabel>
#include <QTextEdit>
#include "ui_importprogressdialog.h"

namespace Ui { class ImportProgressDialog; }

class ImportProgressDialog : public QDialog {
    Q_OBJECT
public:
    explicit ImportProgressDialog(QWidget *parent = nullptr);

public slots:
    void setTotalFiles(int total);
    void onProgress(int processed, int total);
    void onSuccess(int count);
    void onError(const QString &filename, const QString &msg);
    void onFinished(int total, int success, int errors, const QStringList &errorFiles);

private:
    Ui::ImportProgressDialog *ui;
    int successCount = 0;
    int errorCount   = 0;
};

#endif
