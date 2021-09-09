#include "tfzip2.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    TFZip w;
    w.show();

    return a.exec();
}
