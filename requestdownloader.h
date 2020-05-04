#ifndef INPUTSTATEANDREQUEST_H
#define INPUTSTATEANDREQUEST_H

#include <QObject>
#include <QIODevice>
#include "qfcgi/src/qfcgi.h"
#include "parsedrequest.h"

class RequestDownloader : public QObject
{
    Q_OBJECT


    void parseRequest();
public:

    QIODevice *input = nullptr;
    QFCgiRequest *request = nullptr;
    QByteArray requestData;
    int rI = 0;
    int parsed = false;

    explicit RequestDownloader(QIODevice *input, QFCgiRequest *request, int contentLength, QFCgiRequest *parent = nullptr);

    void readAvailableData();

signals:
    void requestParsed(ParsedRequest *parsedRequest);

};

#endif // INPUTSTATEANDREQUEST_H
