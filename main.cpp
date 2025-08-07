#include "mainwindow.h"
#include <QApplication>
#include <QMessageBox>
#include <unistd.h>
#include <QTranslator>
#include <QLocale>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    if (geteuid() != 0) {
        QMessageBox::critical(nullptr, QObject::tr("Error"),
                              QObject::tr("This program requires root privileges. Please restart it using sudo/doas"));
        return 1;
    }
    MainWindow w;
    w.setFixedSize(800,600);
    w.show();
    return a.exec();
}
