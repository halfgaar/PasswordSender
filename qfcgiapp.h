#ifndef QFCGIAPP_H
#define QFCGIAPP_H

#include <QObject>
#include <QCoreApplication>
#include "qfcgi/src/qfcgi.h"
#include "requestdownloader.h"
#include "requestuploader.h"
#include "submittedsecret.h"
#include "emailsender.h"
#include <QHash>
#include "usererror.h"
#include "QTimer"

class QFcgiApp : public QCoreApplication
{
    QFCgi *fcgi = nullptr;
    QHash<QIODevice*, RequestDownloader*> requests;
    QHash<QString, SubmittedSecret_p> submittedSecrets;
    EmailSender emailSender;
    QTimer cleanupTimer;

public:
    QFcgiApp(int argc, char *argv[]);
    ~QFcgiApp();

    void renderReponse(const QString &templateFilePath, const int httpCode, QIODevice *out, const QHash<QString, QString> &templateVariables);

private slots:
    void onNewRequest(QFCgiRequest *request);
    void onReadyRead();
    void onUploadDone();
    void requestParsed(ParsedRequest *parsedRequest);
    void onConnectionClose();
    void onCleanupTimerElapsed();
};

#endif // QFCGIAPP_H
