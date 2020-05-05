#ifndef PARSEDREQUEST_H
#define PARSEDREQUEST_H

#include <QObject>
#include <QVector>
#include <qfcgi/src/qfcgi.h>
#include "uploadedfile.h"
#include "formfield.h"

class ParsedRequest : public QObject
{
    Q_OBJECT
public:
    std::vector<UploadedFile> files;
    QHash<QString,FormField> formFields;
    QHash<QString,QString> params;

    QString scriptURL;
    QFCgiRequest *fcgiRequest = nullptr;

    explicit ParsedRequest(QFCgiRequest *parent);
    void addFile(UploadedFile &uploaded_file);
    void addField(const FormField &formField);

signals:

public slots:
};

#endif // PARSEDREQUEST_H
