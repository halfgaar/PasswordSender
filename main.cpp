#include <QCoreApplication>

#include <QIODevice>
#include <qfcgiapp.h>

int main(int argc, char *argv[])
{
    QFcgiApp a(argc, argv);


    return a.exec();
}
