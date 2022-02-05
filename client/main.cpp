#include "connectwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    ConnectWindow c;
    c.show();
    return a.exec();
}
