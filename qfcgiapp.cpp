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

void QFcgiApp::renderReponse(const QString &templateFilePath, const int httpCode, QIODevice *out, const QHash<QString,QString> &templateVariables)
{
    QFile f(templateFilePath);
    f.open(QFile::ReadOnly);
    QString templateData = QString::fromUtf8(f.readAll());

    for (const QString &key : templateVariables.keys())
    {
        templateData.replace(key, templateVariables[key]);
    }

    QTextStream ts(out);
    ts << "Status: " << httpCode << "\r\n"; // Not HTTP header, but FCGI header.
    ts << "Content-Type: text/html\r\n";
    ts << "\r\n";
    ts << templateData;
    ts.flush();
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

    try
    {
        bool contentLengthAvailable = false;
        int contentLength = request->getParam("CONTENT_LENGTH").toInt(&contentLengthAvailable);

        if (contentLength > (128 * 1024 * 1024))
            throw std::runtime_error("Post-data too large");

        QIODevice *in = request->getIn();
        QIODevice *out = request->getOut();
        connect(in, &QIODevice::readyRead, this, &QFcgiApp::onReadyRead);

        RequestDownloader *downloader = new RequestDownloader(in, request, contentLength);
        connect(downloader, &RequestDownloader::requestParsed, this, &QFcgiApp::requestParsed);
        connect(in, &QIODevice::aboutToClose, this, &QFcgiApp::onConnectionClose);
        connect(out, &QIODevice::aboutToClose, this, &QFcgiApp::onConnectionClose);
        this->requests[in] = downloader;
        downloader->readAvailableData();
    }
    catch (std::exception &ex)
    {
        QHash<QString,QString> vars;
        vars["{errormsg}"] = ex.what();
        renderReponse("/var/www/html/password_sender/errortemplate.html", 500, request->getOut(), vars);
        request->endRequest(1);
    }
}

void QFcgiApp::onReadyRead()
{
    QIODevice *input = static_cast<QIODevice*>(sender());
    RequestDownloader *downloader = this->requests[input];

    try
    {
        downloader->readAvailableData();
    }
    catch (std::exception &ex)
    {
        QHash<QString,QString> vars;
        vars["{errormsg}"] = ex.what();
        renderReponse("/var/www/html/password_sender/errortemplate.html", 500, downloader->request->getOut(), vars);
        downloader->request->endRequest(1);
    }
}

void QFcgiApp::onUploadDone()
{
    RequestUploader *uploader = static_cast<RequestUploader*>(sender());
    uploader->getFcgiRequest()->endRequest(0);
}

void QFcgiApp::requestParsed(ParsedRequest *parsedRequest)
{
    QIODevice *out = parsedRequest->fcgiRequest->getOut();
    try
    {
        if (parsedRequest->scriptURL == "/passwordsender/upload")
        {
            std::shared_ptr<SubmittedSecret> secret(new SubmittedSecret(parsedRequest->formFields["recipient"].value,
                                                    parsedRequest->formFields["password"].value, parsedRequest->files));

            if (!secret->isValid())
            {
                throw std::runtime_error("Secret invalid");
            }

            this->submittedSecrets.insert(secret->uuid, secret);

            QString msg = QString("Informatie gestuurd naar %1").arg(secret->recipient);

            this->emailSender.SendEmail(*secret);

            QHash<QString,QString> vars;
            vars["{message}"] = msg;
            renderReponse("/var/www/html/password_sender/template.html", 200, out, vars);
            parsedRequest->requestDone(0);
        }
        else if (parsedRequest->scriptURL == "/passwordsender/show")
        {
            const QString &uuid = parsedRequest->formFields["uuid"].value;
            std::shared_ptr<SubmittedSecret> secret = this->submittedSecrets[uuid];

            if (!secret)
            {
                throw UserError("Dit geheim bestaat niet (meer). Mogelijk is hij verlopen.");
            }
            else if (!secret->isValid())
            {
                throw std::runtime_error("Secret invalid");
            }

            QString fileLink;

            if (!secret->secretFiles.empty())
            {
                std::shared_ptr<SecretFile> file = *secret->secretFiles.begin(); // TODO: multiple files. The first is just for testing.
                fileLink = QString("<a href=\"%1\">%2</a>").arg(file->getLink()).arg(file->name);
            }

            QHash<QString,QString> vars;
            vars["{secret}"] = secret->passwordField;
            vars["{filelink}"] = fileLink;
            renderReponse("/var/www/html/password_sender/showsecrettemplate.html", 200, out, vars);

            //this->submittedSecrets.remove(uuid); // TODO: I think we need an expire an hour, otherwise you can't download the files.
            parsedRequest->requestDone(0);
        }
        else if (parsedRequest->scriptURL.startsWith("/passwordsender/showlanding/"))
        {
            const QStringList fields = parsedRequest->scriptURL.split('/');
            const QString &uuid = fields[3];

            if (uuid.isEmpty())
            {
                throw UserError("Geen geheim opgegeven. Je zit de boel te flessen.");
            }

            QHash<QString,QString> vars;
            vars["{uuid}"] = uuid;
            renderReponse("/var/www/html/password_sender/showlandingtemplate.html", 200, out, vars);
            parsedRequest->requestDone(0);
        }
        else if (parsedRequest->scriptURL.startsWith("/passwordsender/downloadfile/")) // URL like /passwordsender/downloadfile/[uuid-secret]/[uuid-file]
        {
            const QStringList fields = parsedRequest->scriptURL.split('/');
            const QString &secretUuid = fields[3];
            const QString &fileUuid = fields[4];

            if (secretUuid.isEmpty() || fileUuid.isEmpty())
            {
                throw UserError("Geen geheim of bestand opgegeven. Je zit de boel te flessen.");
            }

            std::shared_ptr<SubmittedSecret> secret = this->submittedSecrets[secretUuid];

            if (!secret)
            {
                throw UserError("Dit geheim bestaat niet (meer). Mogelijk is hij verlopen.");
            }
            else if (!secret->isValid())
            {
                throw std::runtime_error("Secret invalid");
            }

            std::shared_ptr<SecretFile> secretFile = secret->secretFiles[fileUuid];

            QIODevice *out = parsedRequest->fcgiRequest->getOut();
            RequestUploader *uploader = new RequestUploader(out, secretFile, parsedRequest->fcgiRequest);
            connect(uploader, &RequestUploader::uploadDone, this, &QFcgiApp::onUploadDone);
            uploader->uploadNextChunk();
        }
        else
        {
            throw UserError("Pagina bestaat niet.", 404);
        }
    }
    catch (UserError &ex)
    {
        QHash<QString,QString> vars;
        vars["{errormsg}"] = ex.what();
        renderReponse("/var/www/html/password_sender/errortemplate.html", ex.httpCode, out, vars);
        parsedRequest->requestDone(1);
    }
    catch (std::exception)
    {
        QHash<QString,QString> vars;
        vars["{errormsg}"] = "System error";
        renderReponse("/var/www/html/password_sender/errortemplate.html", 500, out, vars);
        parsedRequest->requestDone(1);
    }
}

void QFcgiApp::onConnectionClose()
{
    QIODevice *ioDev = dynamic_cast<QIODevice*>(sender());

    if (this->requests.contains(ioDev))
    {
        this->requests.remove(ioDev);
        ioDev = nullptr;
    }
}














