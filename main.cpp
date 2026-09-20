#include <QApplication>
#include <QMetaType>
#include "mainwindow.h"
#include "editoritem.h"

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);

    qRegisterMetaType<QList<EditorItem>>("QList<EditorItem>");
    qRegisterMetaType<EditorItem>("EditorItem");

    MainWindow w;
    w.show();
    return a.exec();
}
