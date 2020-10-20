#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QApplication::setApplicationName("CurrencyConv");
    QApplication::setOrganizationName("org.keshavnrj.ubuntu");
    QCoreApplication::setApplicationVersion("1");

    MainWindow w;
    w.show();

    return a.exec();
}
