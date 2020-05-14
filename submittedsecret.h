#ifndef SUBMITTEDSECRET_H
#define SUBMITTEDSECRET_H

#include <QString>
#include "parsedrequest.h"
#include <QVector>
#include <QDateTime>
#include "QFile"
#include "QUuid"

class SecretFile;

class SubmittedSecret
{
public:
    QString passwordField;
    QString uuid;
    QList<SecretFile> secretFiles;
    QString recipient;
    QDateTime submittedAt;

    SubmittedSecret(QString &recipient, QString &passwordField, std::vector<UploadedFile> &uploadedFiles);
    QString getLink();
    bool isValid();

};

class SecretFile
{
    SubmittedSecret *parentSecret = nullptr;
    QString name;
    QString uuid;
public:
    SecretFile(SubmittedSecret *parentSecret, UploadedFile &uploadedFile);
    QString getFilePath() const;
};

#endif // SUBMITTEDSECRET_H
