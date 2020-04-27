#ifndef QFCGIAPP_H
#define QFCGIAPP_H

#include <QObject>
#include <QCoreApplication>
#include "qfcgi/src/qfcgi.h"
#include "requestdownloader.h"

class QFcgiApp : public QCoreApplication
{
    QFCgi *fcgi = nullptr;
    QHash<QIODevice*, RequestDownloader*> requests;
public:
    QFcgiApp(int argc, char *argv[]);
    ~QFcgiApp();

private slots:
    void onNewRequest(QFCgiRequest *request);
    void onReadyRead();
};

#endif // QFCGIAPP_H
