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

    std::cout << "Number of attachments: " << parser.getAttachmentCount() << std::endl;

    for (vmime::size_t i = 0; i < parser.getAttachmentCount(); ++i)
    {
        vmime::shared_ptr<const vmime::attachment> att = parser.getAttachmentAt(i);
        std::cout << "name: " << att->getName().getBuffer() << std::endl;

        std::cout << "Type: " << att->getType().getType() << std::endl;

        std::ostringstream dumpFileName(std::ios_base::ate);
        dumpFileName << "/tmp/dumpFile" << i;

        std::cout << "Writing to: " << dumpFileName.str();

        std::ofstream dumpFile(dumpFileName.str());
        vmime::utility::outputStreamAdapter os2(dumpFile);
        att->getData()->extractRaw(os2);
        dumpFile.flush();
        dumpFile.close();

        std::cout << std::endl << std::endl;
    }

    this->request->endRequest(0);

    parsed = true;
}

// TODO: just keeping it in memory for now, but I want to write it to an encrypted temp file.
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
