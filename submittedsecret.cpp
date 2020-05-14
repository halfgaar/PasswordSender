#include "submittedsecret.h"

SubmittedSecret::SubmittedSecret(QString &recipient, QString &passwordField, std::vector<UploadedFile> &uploadedFiles) :
    passwordField(passwordField),
    uuid(QUuid::createUuid().toString().replace('{', "").replace('}',"")), // My Qt version doesn't have the StringFormat option.
    recipient(recipient),
    submittedAt(QDateTime::currentDateTime())
{
    for (UploadedFile &uploadedFile : uploadedFiles)
    {
        SecretFile secretFile(this, uploadedFile);
        this->secretFiles.append(secretFile);
    }
}

QString SubmittedSecret::getLink()
{
    // TODO: don't hardcode hostname
    QString result = QString("https://wachtwoorden.geborsteldstaal.nl/passwordsender/showlanding/%1").arg(this->uuid);
    return result;
}

bool SubmittedSecret::isValid()
{
    return !recipient.isEmpty() && !passwordField.isEmpty();
}

SecretFile::SecretFile(SubmittedSecret *parentSecret, UploadedFile &uploadedFile) :
    parentSecret(parentSecret),
    name(uploadedFile.name),
    uuid(QUuid::createUuid().toString().replace('{', "").replace('}',"")) // My Qt version doesn't have the StringFormat option.
{
    uploadedFile.renameAndrelease(getFilePath());
}

QString SecretFile::getFilePath() const
{
    QString path = QString("/tmp/PasswordSender__secret__%1__file__%2").arg(parentSecret->uuid).arg(uuid);
    return path;
}
