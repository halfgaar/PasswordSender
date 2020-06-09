/**
  * This file is part of PasswordSender.
  *
  * PasswordSender is free software: you can redistribute it and/or modify
  * it under the terms of the GNU General Public License as published by
  * the Free Software Foundation, either version 3 of the License, or
  * (at your option) any later version.

  * PasswordSender is distributed in the hope that it will be useful,
  * but WITHOUT ANY WARRANTY; without even the implied warranty of
  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  * GNU General Public License for more details.
  *
  * You should have received a copy of the GNU General Public License
  * along with PasswordSender. If not, see <https://www.gnu.org/licenses/>.
  *
  * Copyright 2020 Wiebe Cazemier <wiebe@halfgaar.net>
  */

#include "requestuploader.h"
#include "QTextStream"
#include "QDataStream"
#include <iostream>
#include <QDebug>
#include <QTimer>

RequestUploader::RequestUploader(QIODevice *output, std::shared_ptr<SecretFile> &secretFile, QFCgiRequest *request) : QObject(request),
    output(output),
    fileToUpload(secretFile),
    handle(new QFileEncrypted(secretFile->filePath, secretFile->getIv(), secretFile->getCipherKey())),
    request(request)
{
    contentLength = handle->size();

    QTextStream ts(output);
    ts << "Status: " << 200 << "\r\n"; // Not HTTP header, but FCGI header.
    ts << "Content-Disposition: attachment; filename=\"" << secretFile->name.toUtf8().replace('"', "\\") << "\"\r\n";
    ts << "Content-Type: octet-stream\r\n";
    ts << "Content-Length: " << contentLength << "\r\n";
    ts << "\r\n";
    ts.flush();

    if (!handle->open(QFile::ReadOnly))
    {
        QString msg = QString("%1 is not openable.").arg(secretFile->filePath);
        throw std::runtime_error(msg.toStdString());
    }

    timeoutTimer.setInterval(10000);
    connect(&timeoutTimer, &QTimer::timeout, this, &RequestUploader::onTimeout);

    connect(output, &QIODevice::bytesWritten, this, &RequestUploader::onBytesWritten);
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
    QIODevice *out = this->output;

    QByteArray chunk = handle->read(65536);
    out->write(chunk);

    timeoutTimer.start();
}

void RequestUploader::onBytesWritten(qint64 nbytes)
{
    bytesUploaded += nbytes;

    if (bytesUploaded >= contentLength)
    {
        this->timeoutTimer.stop();
        this->handle->close();

        // Make sure this is put at the end of the event queue, otherwise there is a race condition with upstream reading out the socket and us disconnecting it.
        QTimer::singleShot(0, this, &RequestUploader::uploadDone);
    }
    else
        uploadNextChunk();
}

void RequestUploader::onTimeout()
{
    std::cerr << "Timing out upload request" << std::endl;
    this->handle->close();
    this->request->endRequest(1);
}
