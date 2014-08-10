#include <QtGui/QApplication>
#include "errifierui.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    errifierui w;
    w.show();
    
    return a.exec();
}
