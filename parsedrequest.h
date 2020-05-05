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
    std::vector<UploadedFile> files;
    QList<FormField> formFields;

public:
    explicit ParsedRequest(QFCgiRequest *parent = nullptr);
    void addFile(UploadedFile &uploaded_file);
    void addField(const FormField &formField);

signals:

public slots:
};

#endif // PARSEDREQUEST_H
