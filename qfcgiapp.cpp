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
    // You have a new request, this sample implementation will write back
    // a list of all received parameter.

    // request->getOut() is a stream which is used to write back information
    // to the webserver.
    //QTextStream ts(request->getOut());

    //ts << "Content-Type: text/plain\n";
    //ts << "\n";
    //ts << QString("Hello from %1\n").arg(this->applicationName());
    //ts << "This is what I received:\n";

    Q_FOREACH(QString key, request->getParams())
    {
        //ts << QString("%1: %2\n").arg(key).arg(request->getParam(key));
    }

    //ts.flush();

    bool contentLengthAvailable = false;
    int contentLength = request->getParam("CONTENT_LENGTH").toInt(&contentLengthAvailable);
    QString contentType = request->getParam("CONTENT_TYPE");

    if (contentLengthAvailable)
    {
        QIODevice *in = request->getIn();
        connect(in, &QIODevice::readyRead, this, &QFcgiApp::onReadyRead);
        RequestDownloader *downloader = new RequestDownloader(in, request, contentLength, request);
        this->requests[in] = downloader;
        downloader->readAvailableData();
    }

    // Don't forget to call endRequest() to finalize the request
    //request->endRequest(0);
}

void QFcgiApp::onReadyRead()
{
    QIODevice *input = static_cast<QIODevice*>(sender());
    RequestDownloader *downloader = this->requests[input];
    downloader->readAvailableData();


}














