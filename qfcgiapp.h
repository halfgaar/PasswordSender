#ifndef QFCGIAPP_H
#define QFCGIAPP_H

#include <QObject>
#include <QCoreApplication>
#include "qfcgi/src/qfcgi.h"

class QFcgiApp : public QCoreApplication
{
public:
    QFcgiApp(int argc, char *argv[]);
};

#endif // QFCGIAPP_H
