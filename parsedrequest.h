#ifndef PARSEDREQUEST_H
#define PARSEDREQUEST_H

#include <QObject>
#include <QVector>
#include <qfcgi/src/qfcgi.h>
#include "uploadedfile.h"
#include "formfield.h"

#define RAN_BUF_SIZE 32

class ParsedRequest : public QObject
{
    Q_OBJECT
    char *ranBuf = nullptr;

public:
    std::vector<UploadedFile> files;
    QHash<QString,FormField> formFields;
    QHash<QString,QString> params;

    QString scriptURL;
    QString httpHost;
    QFCgiRequest *fcgiRequest = nullptr;

    QByteArray iv = QByteArray(32,0);
    QByteArray cipherKey = QByteArray(32,0);

    explicit ParsedRequest(QFCgiRequest *parent);
    ~ParsedRequest();
    void addFile(UploadedFile &uploaded_file);
    void addField(const FormField &formField);
    void requestDone(quint32 code);

public slots:
};

#endif // PARSEDREQUEST_H
