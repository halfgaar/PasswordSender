#include "requestdownloader.h"
#include <QFile>
#include <QDataStream>
#include <fstream>
#include <sstream>
#include <iostream>

#include "vmime/vmime.hpp"
#include "vmime/contentTypeField.hpp"

RequestDownloader::RequestDownloader(QIODevice *input, QFCgiRequest *request, int contentLength, QFCgiRequest *parent) :
    QObject(parent),
    input(input),
    request(request),
    requestData(contentLength, 0)
{

}

// TODO: can I convert all the c++ streams to QIODevices that run competely non-blocking?
void RequestDownloader::parseRequest()
{
    const QString contentType = QString("%1: %2").arg(vmime::fields::CONTENT_TYPE).arg(this->request->getParam("CONTENT_TYPE"));

    std::fstream rommel("/tmp/rommel", std::ios::trunc | std::ios_base::binary | std::ios_base::in | std::ios_base::out);
    rommel << contentType.toStdString() << std::endl;
    rommel << std::endl;
    rommel << this->requestData.toStdString();
    rommel.flush();

    vmime::shared_ptr<vmime::message> msg = vmime::make_shared<vmime::message>();
    vmime::shared_ptr<vmime::utility::inputStreamAdapter> isa = vmime::make_shared<vmime::utility::inputStreamAdapter>(rommel);
    msg->parse(isa, static_cast<vmime::size_t>(rommel.tellp()));

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

        this->request->endRequest(0);
        parsed = true;
        return;
    }
    else
    {
        this->request->endRequest(1);
        parsed = true;
        return;
    }

    this->request->endRequest(1);
    parsed = true;
}

// TODO: just keeping it in memory for now, but I want to write it to an encrypted temp file.
// TODO: deal with differing content-length and actual post size, both for lost connections and hacks.
void RequestDownloader::readAvailableData()
{
    if (parsed)
        return;

    qint64 n = this->input->bytesAvailable();

    if (n > 0)
    {
        requestData.replace(rI, static_cast<int>(n), input->readAll());

        rI += n;
    }

    Q_ASSERT(rI <= requestData.size());

    if (rI == requestData.size())
    {
        parseRequest();
    }
}
