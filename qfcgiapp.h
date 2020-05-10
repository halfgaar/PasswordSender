#ifndef QFCGIAPP_H
#define QFCGIAPP_H

#include <QObject>
#include <QCoreApplication>
#include "qfcgi/src/qfcgi.h"
#include "requestdownloader.h"
#include "submittedsecret.h"
#include "emailsender.h"
#include <QHash>
#include "usererror.h"

class QFcgiApp : public QCoreApplication
{
    QFCgi *fcgi = nullptr;
    QHash<QIODevice*, RequestDownloader*> requests;
    QHash<QString, std::shared_ptr<SubmittedSecret>> submittedSecrets;
    EmailSender emailSender;

public:
    QFcgiApp(int argc, char *argv[]);
    ~QFcgiApp();

    void renderReponse(const QString &templateFilePath, const int httpCode, QIODevice *out, const QHash<QString, QString> &templateVariables);

private slots:
    void onNewRequest(QFCgiRequest *request);
    void onReadyRead();
    void requestParsed(ParsedRequest *parsedRequest);
};

#endif // QFCGIAPP_H
