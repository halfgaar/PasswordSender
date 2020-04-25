#ifndef QFCGIAPP_H
#define QFCGIAPP_H

#include <QObject>
#include <QCoreApplication>
#include "qfcgi/src/qfcgi.h"

class QFcgiApp : public QCoreApplication
{
    QFCgi *fcgi = nullptr;
public:
    QFcgiApp(int argc, char *argv[]);
    ~QFcgiApp();

private slots:
    void onNewRequest(QFCgiRequest *request);
};

#endif // QFCGIAPP_H
