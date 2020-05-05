#include "submittedsecret.h"

SubmittedSecret::SubmittedSecret(QString &passwordField, const std::vector<UploadedFile> &uploadedFiles) :
    passwordField(passwordField),
    uuid(QUuid::createUuid().toString())
{
    for (const UploadedFile &uploadedFile : uploadedFiles)
    {
        SecretFile secretFile(this, uploadedFile);
        this->secretFiles.append(secretFile);
    }
}

SecretFile::SecretFile(SubmittedSecret *parentSecret, const UploadedFile &uploadedFile) :
    parentSecret(parentSecret),
    name(uploadedFile.name)
{
    QFile::copy(uploadedFile.pathToDataFile, getFilePath());

}

QString SecretFile::getFilePath() const
{
    QString path = QString("/tmp/PasswordSender_secret_%1").arg(parentSecret->uuid);
    return path;
}
