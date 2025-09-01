#include "../include/mainwindow.h"
#include "../include/logger.hpp"


#include <QApplication>
#include <QMessageBox>
#include <QTranslator>
#include <QLocale>

#include <unistd.h>

int main(int argc, char *argv[])
{

    logL("Application has started!");
    QApplication a(argc, argv);
    if (geteuid() != 0) {
        logF("Main: No Root Priveleges Granted!");
        QMessageBox::critical(nullptr, QObject::tr("Error"),
                              QObject::tr("This program requires root privileges. Please restart it using sudo/doas"));

        return 1;
    }
    if (qEnvironmentVariableIsEmpty("XDG_RUNTIME_DIR")) {
        QString runtimeDir = QString("/run/user/%1").arg(getuid());
        if (QDir(runtimeDir).exists()) {
            qputenv("XDG_RUNTIME_DIR", runtimeDir.toUtf8());
        }
    }
    MainWindow w;
    //w.setFixedSize(800,700);
    w.show();
    return a.exec();
}
