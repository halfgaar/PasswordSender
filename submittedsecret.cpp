#include "submittedsecret.h"

SubmittedSecret::SubmittedSecret(QString &recipient, QString &passwordField, const std::vector<UploadedFile> &uploadedFiles) :
    passwordField(passwordField),
    uuid(QUuid::createUuid().toString().replace('{', "").replace('}',"")), // My Qt version doesn't have the StringFormat option.
    recipient(recipient)
{
    for (const UploadedFile &uploadedFile : uploadedFiles)
    {
        SecretFile secretFile(this, uploadedFile);
        this->secretFiles.append(secretFile);
    }
}

QString SubmittedSecret::getLink()
{
    // TODO: don't hardcode hostname
    QString result = QString("https://wachtwoorden.geborsteldstaal.nl/passwordsender/show/%1").arg(this->uuid);
    return result;
}

bool SubmittedSecret::isValid()
{
    return !recipient.isEmpty() && !passwordField.isEmpty();
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
