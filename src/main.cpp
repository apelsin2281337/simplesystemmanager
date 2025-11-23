#include "../include/mainwindow.h"
#include "../include/logger.hpp"


#include <QApplication>
#include <QMessageBox>
#include <QTranslator>
#include <QLocale>
#include <QDir>

#include <unistd.h>

int main(int argc, char *argv[])
{
    

    logL("Application has started!");
    QApplication a(argc, argv);
    if (geteuid() != 0) {
        logF("Main: No Root Privileges Granted!");
        QMessageBox::critical(nullptr, QObject::tr("Error"),
                              QObject::tr("This program requires root privileges. Please restart it using sudo/doas"));

        return 1;
    }
    QTranslator translator;
    if (translator.load(QLocale(), "qtguiinterface", "_", ":/translations")) {
        QCoreApplication::installTranslator(&translator);
    }
    MainWindow w;
    w.show();
    return a.exec();
}
