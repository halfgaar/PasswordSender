#include "requestdownloader.h"
#include <QFile>
#include <QDataStream>
#include <fstream>
#include <sstream>
#include <iostream>

#include "vmime/vmime.hpp"
#include "vmime/contentTypeField.hpp"

RequestDownloader::RequestDownloader(QIODevice *input, QFCgiRequest *request, int contentLength) :
    QObject(request),
    input(input),
    request(request)
{
    contentType = QString("%1: %2").arg(vmime::fields::CONTENT_TYPE).arg(this->request->getParam("CONTENT_TYPE"));
    QByteArray header = QString("%1\r\n\r\n").arg(this->contentType).toUtf8();

    // Adding the header to the body and adding to the content length because vmime wants the content length header as part of the body. That
    // is, it has its own concept of header and body.
    rI = header.length();
    int len = contentLength + header.length();
    requestData.reset(new QByteArray(len, 0));

    requestData->replace(0, header.length(), header);

    timeoutTimer.setInterval(10000);
    connect(&timeoutTimer, &QTimer::timeout, this, &RequestDownloader::onTimeout);
}

// TODO: can I convert all the c++ streams to QIODevices that run competely non-blocking?
void RequestDownloader::parseRequest()
{
    vmime::shared_ptr<vmime::utility::inputStreamStringAdapter> isa = vmime::make_shared<vmime::utility::inputStreamStringAdapter>(this->requestData->toStdString());

    vmime::shared_ptr<vmime::message> msg = vmime::make_shared<vmime::message>();
    msg->parse(isa, static_cast<size_t>(this->requestData->length()));

    vmime::messageParser parser(msg);

    if (parser.getAttachmentCount() > 0)
    {
        ParsedRequest parsedRequest(this->request);

        for (vmime::size_t i = 0; i < parser.getAttachmentCount(); ++i)
        {
            vmime::shared_ptr<const vmime::attachment> att = parser.getAttachmentAt(i);
            const std::string fileName = att->getName().getBuffer(); //vmime thinks in e-mail terms, and 'name' is taken from 'filename', not 'name'.

            if (!fileName.empty()) // Uploaded files
            {
                UploadedFile uploadedFile(fileName, att);
                parsedRequest.addFile(uploadedFile);
            }
            else // form fields
            {
                vmime::shared_ptr<vmime::parameterizedHeaderField> contentDisposition =
                        att->getHeader()->findField<vmime::parameterizedHeaderField>(vmime::fields::CONTENT_DISPOSITION);

                if (contentDisposition)
                {
                    vmime::shared_ptr<vmime::parameter> nameParamter = contentDisposition->findParameter("name");
                    if (nameParamter)
                    {
                        std::string name = nameParamter->getValue().getBuffer();
                        FormField formField(name, att);
                        parsedRequest.addField(formField);
                    }

                }
            }

            std::cout << std::endl << std::endl;
        }

        // TODO: decide this: because the slot is in the same thread, it will execute as if a function all, and parsedRequest is cleaned up when
        // going out of scope, right after having ended the request. If we ever make the app multi threaded, we need to make sure requests
        // stay within the thread, to avoid having to put endrequest() to some signal. So, signals may not be the logical choice. A
        // Simpel callback perhaps?
        emit requestParsed(&parsedRequest);
        parsed = true;
    }
    else
    {
        this->request->endRequest(1);
        parsed = true;
    }

    parsed = true;
}

void RequestDownloader::onTimeout()
{
    std::cerr << "Timing out request" << std::endl;
    this->request->endRequest(1);
}

// TODO: just keeping it in memory for now, but I want to write it to an encrypted temp file.
void RequestDownloader::readAvailableData()
{
    if (parsed)
        return;

    this->timeoutTimer.start();

    qint64 n = this->input->bytesAvailable();

    if (n > (requestData->size() - rI) )
        throw std::runtime_error("More bytes than Content-Length");

    if (n > 0)
    {
        requestData->replace(rI, static_cast<int>(n), input->readAll());

        rI += n;
    }

    Q_ASSERT(rI <= requestData->size());

    if (rI == requestData->size())
    {
        this->timeoutTimer.stop();
        parseRequest();
    }
}
