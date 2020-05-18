#include "submittedsecret.h"

SubmittedSecret::SubmittedSecret(ParsedRequest *parsedRequest) :
    uuid(QUuid::createUuid().toString().replace('{', "").replace('}',"")), // My Qt version doesn't have the StringFormat option.
    submittedAt(QDateTime::currentDateTime())
{
    recipient = parsedRequest->formFields["recipient"].value;
    passwordField = parsedRequest->formFields["password"].value;
    iv = parsedRequest->iv;
    cipherKey = parsedRequest->cipherKey;

    for (UploadedFile &uploadedFile : parsedRequest->files)
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

QByteArray SecretFile::getIv()
{
    return parentSecret->iv;
}

QByteArray SecretFile::getCipherKey()
{
    return parentSecret->cipherKey;
}
