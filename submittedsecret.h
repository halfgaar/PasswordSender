#ifndef SUBMITTEDSECRET_H
#define SUBMITTEDSECRET_H

#include <QString>
#include "parsedrequest.h"
#include <QVector>
#include "QFile"
#include "QUuid"

class SecretFile;

class SubmittedSecret
{
public:
    QString passwordField;
    QString uuid;
    QList<SecretFile> secretFiles;

    SubmittedSecret(QString &passwordField, const std::vector<UploadedFile> &uploadedFiles);

};

class SecretFile
{
    SubmittedSecret *parentSecret = nullptr;
    QString name;
    QString sanitizedName;
public:
    SecretFile(SubmittedSecret *parentSecret, const UploadedFile &uploadedFile);
    QString getFilePath() const;
};

#endif // SUBMITTEDSECRET_H
