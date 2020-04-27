#ifndef INPUTSTATEANDREQUEST_H
#define INPUTSTATEANDREQUEST_H

#include <QObject>
#include <QIODevice>
#include "qfcgi/src/qfcgi.h"

class RequestDownloader : public QObject
{
    Q_OBJECT

public:

    QIODevice *input = nullptr;
    QFCgiRequest *request = nullptr;
    QByteArray requestData;
    int rI = 0;

    explicit RequestDownloader(QIODevice *input, QFCgiRequest *request, int contentLength, QObject *parent = nullptr);

    void readAvailableData();

};

#endif // INPUTSTATEANDREQUEST_H
