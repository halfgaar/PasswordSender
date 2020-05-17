#ifndef SUBMITTEDSECRET_H
#define SUBMITTEDSECRET_H

#include <QString>
#include "parsedrequest.h"
#include <QVector>
#include <QDateTime>
#include "QFile"
#include "QUuid"

class SecretFile;

/**
 * @brief The SubmittedSecret class is the memory structure of a secret in memory, waiting for someone to download it. Because
 * the class also manages SecretFiles, it's non copyable or movable. Used smart pointers to manage.
 */
class SubmittedSecret
{
public:
    QString passwordField;
    QString uuid;
    QHash<QString,std::shared_ptr<SecretFile>> secretFiles;
    QString recipient;
    QDateTime submittedAt;

    SubmittedSecret(QString &recipient, QString &passwordField, std::vector<UploadedFile> &uploadedFiles);
    SubmittedSecret(const SubmittedSecret &other) = delete;
    SubmittedSecret(SubmittedSecret &&other) = delete;
    ~SubmittedSecret();
    SubmittedSecret & operator=(const SubmittedSecret&) = delete;
    QString getLink();
    bool isValid();
};

/**
 * @brief The SecretFile class is a file of a secret. Because the class also manages a file, it's non copyable or movable. Used smart pointers to manage.
 */
class SecretFile
{
    SubmittedSecret *parentSecret = nullptr;
public:
    QString name;
    QString uuid;
    QString filePath;

    SecretFile(SubmittedSecret *parentSecret, UploadedFile &uploadedFile);
    SecretFile(const SecretFile &other) = delete;
    SecretFile(SecretFile &&other) = delete;
    ~SecretFile();
    SecretFile & operator=(const SecretFile&) = delete;
    QString getLink();
};

#endif // SUBMITTEDSECRET_H
