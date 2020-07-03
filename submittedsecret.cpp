/**
  * This file is part of PasswordSender.
  *
  * PasswordSender is free software: you can redistribute it and/or modify
  * it under the terms of the GNU General Public License as published by
  * the Free Software Foundation, either version 3 of the License, or
  * (at your option) any later version.

  * PasswordSender is distributed in the hope that it will be useful,
  * but WITHOUT ANY WARRANTY; without even the implied warranty of
  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  * GNU General Public License for more details.
  *
  * You should have received a copy of the GNU General Public License
  * along with PasswordSender. If not, see <https://www.gnu.org/licenses/>.
  *
  * Copyright 2020 Wiebe Cazemier <wiebe@halfgaar.net>
  */

#include "submittedsecret.h"
#include <QRegularExpression>

SubmittedSecret::SubmittedSecret(ParsedRequest *parsedRequest) :
    uuid(QUuid::createUuid().toString().replace('{', "").replace('}',"")), // My Qt version doesn't have the StringFormat option.
    submittedAt(QDateTime::currentDateTime()),
    expiresAt(submittedAt.addDays(3))
{
    recipient = parsedRequest->formFields["recipient"].value;
    passwordField = parsedRequest->formFields["password"].value;
    iv = parsedRequest->iv;
    cipherKey = parsedRequest->cipherKey;
    httpHost = parsedRequest->httpHost;

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
    QString result = QString("https://%1/passwordsender/showlanding/%2").arg(httpHost).arg(this->uuid);
    return result;
}

bool SubmittedSecret::isValid()
{
    QRegularExpression r(".+@.+\\..+");

    return !recipient.isEmpty() && !passwordField.isEmpty() && r.match(recipient).hasMatch();
}

void SubmittedSecret::expireSoon()
{
    if (!seen)
    {
        seen = true;
        expiresAt = QDateTime::currentDateTime().addSecs(3600);
    }
}

bool SubmittedSecret::hasExpired()
{
    return expiresAt < QDateTime::currentDateTime();
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
    QString result = QString("https://%1/passwordsender/downloadfile/%2/%3").arg(parentSecret->httpHost).arg(parentSecret->uuid).arg(this->uuid);
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
