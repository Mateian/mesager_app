#include "TCPApp.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    TCPApp window;
    window.show();
    return app.exec();
}
