#include "qfcgiapp.h"
#include "QTextStream"
#include "QHostAddress"
#include "QTimer"
#include "QThread"

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

    if (contentLengthAvailable)
    {
        QIODevice *in = request->getIn();
        connect(in, &QIODevice::readyRead, this, &QFcgiApp::onReadyRead);

        RequestDownloader *downloader = new RequestDownloader(in, request, contentLength, request);
        connect(downloader, &RequestDownloader::requestParsed, this, &QFcgiApp::requestParsed);
        this->requests[in] = downloader;
        downloader->readAvailableData();
    }
}

void QFcgiApp::onReadyRead()
{
    QIODevice *input = static_cast<QIODevice*>(sender());
    RequestDownloader *downloader = this->requests[input];
    downloader->readAvailableData();
}

void QFcgiApp::requestParsed(ParsedRequest *parsedRequest)
{
    QIODevice *out = parsedRequest->fcgiRequest->getOut();
    QTextStream ts(out);
    ts << "Content-Type: text/plain\n";
    ts << "\n";

    if (parsedRequest->scriptURL.startsWith("/upload"))
    {
        //TODO: error handling/handler. formfield may be default constructed one.

        SubmittedSecret secret(parsedRequest->formFields["password"].value, parsedRequest->files);
        this->submittedSecrets[secret.uuid] = secret;
    }
}














