#include "qfcgiapp.h"
#include "QTextStream"
#include "QHostAddress"
#include "QTimer"

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
    QTextStream ts(request->getOut());

    ts << "Content-Type: plain/text\n";
    ts << "\n";
    ts << QString("Hello from %1\n").arg(this->applicationName());
    ts << "This is what I received:\n";

    Q_FOREACH(QString key, request->getParams()) {
      ts << QString("%1: %2\n").arg(key).arg(request->getParam(key));
    }

    ts.flush();

    // Don't forget to call endRequest() to finalize the request
    request->endRequest(0);
}
