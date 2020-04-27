#include "requestdownloader.h"

RequestDownloader::RequestDownloader(QIODevice *input, QFCgiRequest *request, int contentLength, QObject *parent) :
    QObject(parent),
    input(input),
    request(request),
    requestData(contentLength, 0)
{

}

// TODO: just keeping it in memory for now, but I want to write it to an encrypted temp file.
void RequestDownloader::readAvailableData()
{
    qint64 n = this->input->bytesAvailable();

    if (n > 0)
    {
        requestData.replace(rI, static_cast<int>(n), input->readAll());

        rI += n;
    }
}
