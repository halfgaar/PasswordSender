#include "requestuploader.h"
#include "QTextStream"
#include "QDataStream"
#include <iostream>

RequestUploader::RequestUploader(QIODevice *output, std::shared_ptr<SecretFile> &secretFile, QFCgiRequest *request) : QObject(request),
    output(output),
    fileToUpload(secretFile),
    handle(new QFile(secretFile->filePath)),
    request(request)
{
    qint64 len = handle->size();

    QTextStream ts(output);
    ts << "Status: " << 200 << "\r\n"; // Not HTTP header, but FCGI header.
    ts << "Content-Disposition: attachment; filename=\"" << secretFile->name.toUtf8().replace('"', "\\") << "\"\r\n";
    ts << "Content-Type: octet-stream\r\n";
    ts << "Content-Length: " << len << "\r\n";
    ts << "\r\n";
    ts.flush();

    if (!handle->open(QFile::ReadOnly))
    {
        QString msg = QString("%1 is not openable.").arg(secretFile->filePath);
        throw std::runtime_error(msg.toStdString());
    }

    timeoutTimer.setInterval(10000);
    connect(&timeoutTimer, &QTimer::timeout, this, &RequestUploader::onTimeout);

    connect(output, &QFile::bytesWritten, this, &RequestUploader::onBytesWritten);
}

/**
 * @brief RequestUploader::uploadNextChunk uploads the next chunk to the webserver
 *
 * Note that it has a chunk size of 64k. Qfcgi actually ignores the size of the chunk and always writes 64k, so we have to do that. Also,
 * qfcgi actually wrongly assumes the whole chunk is written. It misses logic of actually checking the amount of bytes written, and continuing
 * until they are all written. I think it's more of a coincidence that it just works with 64k.
 */
void RequestUploader::uploadNextChunk()
{
    QFile *in = this->handle.get();
    QIODevice *out = this->output;

    QByteArray chunk = in->read(65536);
    out->write(chunk);

    timeoutTimer.start();
}

void RequestUploader::onBytesWritten(qint64 nbytes)
{
    Q_UNUSED(nbytes)

    if (this->handle->atEnd())
    {
        this->timeoutTimer.stop();
        emit uploadDone();
    }
    else
        uploadNextChunk();
}

void RequestUploader::onTimeout()
{
    std::cerr << "Timing out upload request" << std::endl;
    this->request->endRequest(1);
}
