#include "mastrview.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MastrView w;
    w.show();
    return a.exec();
}
