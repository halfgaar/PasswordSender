#include "submittedsecret.h"

SubmittedSecret::SubmittedSecret(QString &recipient, QString &passwordField, std::vector<UploadedFile> &uploadedFiles) :
    passwordField(passwordField),
    uuid(QUuid::createUuid().toString().replace('{', "").replace('}',"")), // My Qt version doesn't have the StringFormat option.
    recipient(recipient),
    submittedAt(QDateTime::currentDateTime())
{
    for (UploadedFile &uploadedFile : uploadedFiles)
    {
        std::shared_ptr<SecretFile> s(new SecretFile(this, uploadedFile));
        this->secretFiles[s->uuid] = s;
    }
}

SubmittedSecret::~SubmittedSecret()
{

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
    uuid(QUuid::createUuid().toString().replace('{', "").replace('}',"")), // My Qt version doesn't have the StringFormat option.
    filePath(QString("/tmp/PasswordSender__secret__%1__file__%2").arg(parentSecret->uuid).arg(uuid))
{
    uploadedFile.renameAndrelease(this->filePath);
}

SecretFile::~SecretFile()
{
    if (!this->filePath.isEmpty() && QFile::exists(this->filePath))
    {
        QFile::remove(this->filePath);
    }
}

QString SecretFile::getLink()
{
    // TODO: don't hardcode hostname
    QString result = QString("https://wachtwoorden.geborsteldstaal.nl/passwordsender/downloadfile/%1/%2").arg(parentSecret->uuid).arg(this->uuid);
    return result;
}
