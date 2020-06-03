#include "qfcgiapp.h"
#include "QTextStream"
#include "QHostAddress"
#include "QTimer"
#include "QThread"
#include <QString>
#include "iostream"
#include <QCommandLineParser>
#include <QDir>

QFcgiApp::QFcgiApp(int argc, char *argv[]) : QCoreApplication(argc, argv)
{
    QCoreApplication::setApplicationName("PasswordSender");

    QCommandLineParser parser;
    parser.setApplicationDescription("FastCGI app for hosting a little webapp to send secrets.");
    parser.addHelpOption();

    parser.addOption({"listen-port", "The port to listen on. Always localhost. Default 9000.", "port", "9000"});
    parser.addOption({"template-dir", "The dir with the templates. Should not be in the docroot of the webserver.", "dir"});

    parser.process(*this);

    bool isInt = false;
    uint port = parser.value("listen-port").toUInt(&isInt);
    if (!isInt)
    {
        qCritical("Port number is not a number.");
        return;
    }

    templateDir = QDir::cleanPath(parser.value("template-dir"));
    QDir dir(templateDir);
    if (templateDir.isEmpty())
    {
        qCritical() << "Template dir is not given.";
        return;
    }
    if (!dir.exists(templateDir) || !dir.isReadable())
    {
        qCritical() << "Template dir " << templateDir << "not found or not readable.";
        return;
    }

    fcgi = new QFCgi(this);
    connect(fcgi, &QFCgi::newRequest, this, &QFcgiApp::onNewRequest);

    fcgi->configureListen(QHostAddress::Any, port);

    this->fcgi->start();

    if (!this->fcgi->isStarted())
    {
        qCritical() << this->fcgi->errorString();
        return;
    }

    cleanupTimer.setInterval(1000*60);
    cleanupTimer.setSingleShot(false);
    connect(&cleanupTimer, &QTimer::timeout, this, &QFcgiApp::onCleanupTimerElapsed);
    cleanupTimer.start();

    initialized = true;
}

QFcgiApp::~QFcgiApp()
{

}

void QFcgiApp::renderReponse(const QString &templateFileName, const int httpCode, QIODevice *out, const QHash<QString,QString> &templateVariables)
{
    const QString templateFilePath = QDir::cleanPath(templateDir + QDir::separator() + templateFileName);
    QFile f(templateFilePath);
    if (!f.open(QFile::ReadOnly))
    {
        QString msg = QString("Can't open template %1").arg(templateFilePath);
        throw std::runtime_error(msg.toStdString());
    }
    QString templateData = QString::fromUtf8(f.readAll());

    for (const QString &key : templateVariables.keys())
    {
        templateData.replace(key, templateVariables[key]);
    }

    QTextStream ts(out);
    ts << "Status: " << httpCode << "\r\n"; // Not HTTP header, but FCGI header.
    ts << "Content-Type: text/html\r\n";
    ts << "Cache-Control: no-store\r\n";
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
        renderReponse("errortemplate.html", 500, request->getOut(), vars);
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
        renderReponse("errortemplate.html", 500, downloader->request->getOut(), vars);
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
    const auto &submittedSecretsConst = submittedSecrets;
    try
    {
        if (parsedRequest->scriptURL == "/passwordsender/upload")
        {
            SubmittedSecret_p secret(new SubmittedSecret(parsedRequest));

            if (!secret->isValid())
            {
                throw std::runtime_error("Secret invalid");
            }

            this->submittedSecrets.insert(secret->uuid, secret);

            QString msg = QString("Informatie gestuurd naar %1").arg(secret->recipient);

            this->emailSender.SendEmail(*secret);

            QHash<QString,QString> vars;
            vars["{message}"] = msg;
            renderReponse("template.html", 200, out, vars);
            parsedRequest->requestDone(0);
        }
        else if (parsedRequest->scriptURL == "/passwordsender/show")
        {
            const QString &uuid = parsedRequest->formFields["uuid"].value;
            SubmittedSecret_p secret = submittedSecretsConst[uuid];

            if (!secret)
            {
                throw UserError("Dit geheim bestaat niet (meer). Mogelijk is hij verlopen.", 404);
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
            renderReponse("showsecrettemplate.html", 200, out, vars);

            secret->expireSoon();
            parsedRequest->requestDone(0);
        }
        else if (parsedRequest->scriptURL.startsWith("/passwordsender/showlanding/"))
        {
            const QStringList fields = parsedRequest->scriptURL.split('/');
            const QString &uuid = fields[3];

            if (uuid.isEmpty())
            {
                throw UserError("Geen geheim opgegeven. Je zit de boel te flessen.", 500);
            }

            QHash<QString,QString> vars;
            vars["{uuid}"] = uuid;
            renderReponse("showlandingtemplate.html", 200, out, vars);
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

            SubmittedSecret_p secret = submittedSecretsConst[secretUuid];

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
        renderReponse("errortemplate.html", ex.httpCode, out, vars);
        parsedRequest->requestDone(1);
    }
    catch (std::exception)
    {
        QHash<QString,QString> vars;
        vars["{errormsg}"] = "System error";
        renderReponse("errortemplate.html", 500, out, vars);
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

void QFcgiApp::onCleanupTimerElapsed()
{
    auto i = submittedSecrets.begin();
    while(i != submittedSecrets.end())
    {
        SubmittedSecret_p p = *i;
        if (p && p->hasExpired())
        {
            i = submittedSecrets.erase(i);
        }
        else
            i++;
    }
}














