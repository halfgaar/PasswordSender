#ifndef REQUESTUPLOADER_H
#define REQUESTUPLOADER_H

#include <QObject>
#include "qfcgi/src/qfcgi.h"
#include "submittedsecret.h"
#include <qfileencrypted.h>
#include <QTimer>

/**
 * @brief The RequestUploader class uploads a file back to the user.
 *
 * Life and destruction is managed by the parent, QFCgiRequest.
 */
class RequestUploader : public QObject
{
    Q_OBJECT
    QIODevice *output;
    std::shared_ptr<SecretFile> fileToUpload;
    std::shared_ptr<QFileEncrypted> handle;
    QTimer timeoutTimer;
    QFCgiRequest *request = nullptr;
    qint64 bytesUploaded = 0;
    qint64 contentLength = 0;

private slots:
    void onBytesWritten(qint64 nbytes);
    void onTimeout();

public:
    explicit RequestUploader(QIODevice *output, std::shared_ptr<SecretFile> &secretFile, QFCgiRequest *request);
    RequestUploader() = delete;
    RequestUploader(const RequestUploader &other) = delete;
    RequestUploader & operator=(const RequestUploader &other) = delete;
    void uploadNextChunk();
    QFCgiRequest *getFcgiRequest() { return this->request; }
    qint64 getContentLength() { return this->contentLength; }

signals:
    void uploadDone();

public slots:
};

#endif // REQUESTUPLOADER_H
