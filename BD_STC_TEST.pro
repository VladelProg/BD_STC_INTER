QT       += core gui sql widgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++14

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    databasemanager.cpp \
    editdialog.cpp \
    editoritem.cpp \
    editortablemodel.cpp \
    importprogressdialog.cpp \
    importworker.cpp \
    main.cpp \
    mainwindow.cpp

HEADERS += \
    databasemanager.h \
    editdialog.h \
    editoritem.h \
    editortablemodel.h \
    importprogressdialog.h \
    importworker.h \
    mainwindow.h

FORMS += \
    editdialog.ui \
    importprogressdialog.ui \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

#DISTFILES += \
#    model.qmodel
