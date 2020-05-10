#include "submittedsecret.h"

SubmittedSecret::SubmittedSecret(QString &passwordField, const std::vector<UploadedFile> &uploadedFiles) :
    passwordField(passwordField),
    uuid(QUuid::createUuid().toString().replace('{', "").replace('}',"")) // My Qt version doesn't have the StringFormat option.
{
    for (const UploadedFile &uploadedFile : uploadedFiles)
    {
        SecretFile secretFile(this, uploadedFile);
        this->secretFiles.append(secretFile);
    }
}

QString sanitizeNameForDisk(const QString &input)
{
    QString result;
    for (QChar c : input)
    {
        result.append(c);

        // TODO
    }

    return result;
}

SecretFile::SecretFile(SubmittedSecret *parentSecret, const UploadedFile &uploadedFile) :
    parentSecret(parentSecret),
    name(uploadedFile.name)
{
    QFile::copy(uploadedFile.pathToDataFile, getFilePath());
}

QString SecretFile::getFilePath() const
{
    QString path = QString("/tmp/PasswordSender__secret__%1__%2").arg(parentSecret->uuid);
    return path;
}
