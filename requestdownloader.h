#ifndef INPUTSTATEANDREQUEST_H
#define INPUTSTATEANDREQUEST_H

#include <QObject>
#include <QIODevice>
#include <QTimer>
#include "qfcgi/src/qfcgi.h"
#include "parsedrequest.h"

#include "uploadedfile.h"
#include "formfield.h"

class RequestDownloader : public QObject
{
    Q_OBJECT

    void parseRequest();

private slots:
    void onTimeout();

public:

    QIODevice *input = nullptr;
    QFCgiRequest *request = nullptr;
    std::unique_ptr<QByteArray> requestData;
    int rI = 0;
    int parsed = false;
    QString contentType;
    QTimer timeoutTimer;

    explicit RequestDownloader(QIODevice *input, QFCgiRequest *request, int contentLength, QFCgiRequest *parent);

    void readAvailableData();

signals:
    void requestParsed(ParsedRequest *parsedRequest);

};

#endif // INPUTSTATEANDREQUEST_H
