#include "qfcgiapp.h"
#include "QTextStream"
#include "QHostAddress"
#include "QTimer"
#include "QThread"
#include <QString>
#include "iostream"

QFcgiApp::QFcgiApp(int argc, char *argv[]) : QCoreApplication(argc, argv)
{
    fcgi = new QFCgi(this);
    connect(fcgi, &QFCgi::newRequest, this, &QFcgiApp::onNewRequest);

    fcgi->configureListen(QHostAddress::Any, 9000);

    this->fcgi->start();

    if (!this->fcgi->isStarted())
    {
      qCritical() << this->fcgi->errorString();
      QTimer::singleShot(0, this, &QFcgiApp::quit);
    }
}

QFcgiApp::~QFcgiApp()
{

}

void QFcgiApp::onNewRequest(QFCgiRequest *request)
{
    // request->getOut() is a stream which is used to write back information
    // to the webserver.
    //QTextStream ts(request->getOut());

    //ts << "Content-Type: text/plain\n";
    //ts << "\n";
    //ts << QString("Hello from %1\n").arg(this->applicationName());
    //ts << "This is what I received:\n";

    bool contentLengthAvailable = false;
    int contentLength = request->getParam("CONTENT_LENGTH").toInt(&contentLengthAvailable);

    QIODevice *in = request->getIn();
    connect(in, &QIODevice::readyRead, this, &QFcgiApp::onReadyRead);

    RequestDownloader *downloader = new RequestDownloader(in, request, contentLength, request);
    connect(downloader, &RequestDownloader::requestParsed, this, &QFcgiApp::requestParsed);
    this->requests[in] = downloader;
    downloader->readAvailableData();
}

void QFcgiApp::onReadyRead()
{
    QIODevice *input = static_cast<QIODevice*>(sender());
    RequestDownloader *downloader = this->requests[input];
    downloader->readAvailableData();
}

// TODO: generic error handler with exceptions and error template?
void QFcgiApp::requestParsed(ParsedRequest *parsedRequest)
{
    QIODevice *out = parsedRequest->fcgiRequest->getOut();
    QTextStream ts(out);
    ts << "Content-Type: text/html\n";
    ts << "\n";

    if (parsedRequest->scriptURL.startsWith("/passwordsender/upload"))
    {
        QString recipient = parsedRequest->formFields["recipient"].value;
        if (recipient.isEmpty())
        {
            ts.flush();
            return;
        }

        std::shared_ptr<SubmittedSecret> secret(new SubmittedSecret(parsedRequest->formFields["password"].value, parsedRequest->files));
        this->submittedSecrets.insert(secret->uuid, secret);

        QString url = QString("http://bla.halfgaar.net/passwordsender/show/%1").arg(secret->uuid);
        std::cout << "Download url: " << url.toStdString() << std::endl;

        QString msg = QString("Informatie gestuurd naar %1").arg(recipient);

        QFile templateHtml("/var/www/html/password_sender/template.html");
        templateHtml.open(QFile::ReadOnly);
        QString templateContent = QString::fromUtf8(templateHtml.readAll());
        templateContent.replace("{message}", msg);

        ts << templateContent;
        ts.flush();
        return;
    }
    else if (parsedRequest->scriptURL.startsWith("/passwordsender/show/"))
    {
        const QStringList fields = parsedRequest->scriptURL.split('/');
        const QString &uuid = fields[3];

        if (uuid.length() == 0)
        {
            ts << "Go away" << "\n";
            ts.flush();
            return;
        }
        else
        {
            std::shared_ptr<SubmittedSecret> &secret = this->submittedSecrets[uuid];
            if (secret)
            {
                ts << "Password: " << secret->passwordField << "\n";
                ts.flush();
                return;
            }
            else
            {
                ts << "No secret by that UUID" << "\n";
                ts.flush();
                return;
            }
        }
    }
    else if (parsedRequest->scriptURL.startsWith("/passwordsender/downloadfile/"))
    {
        // TODO: serve file from disk, decrypting on the fly. Just setting headers and go?
    }
}














