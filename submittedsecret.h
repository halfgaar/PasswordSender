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
    QDateTime expiresAt;
    bool seen = false;
    QByteArray iv;
    QByteArray cipherKey;
    QString httpHost;

    SubmittedSecret(ParsedRequest *parsedRequest);
    SubmittedSecret(const SubmittedSecret &other) = delete;
    SubmittedSecret(SubmittedSecret &&other) = delete;
    ~SubmittedSecret();
    SubmittedSecret & operator=(const SubmittedSecret&) = delete;
    QString getLink();
    bool isValid();
    void expireSoon();
    bool hasExpired();
};

typedef std::shared_ptr<SubmittedSecret> SubmittedSecret_p;

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
    QByteArray getIv();
    QByteArray getCipherKey();
};

#endif // SUBMITTEDSECRET_H
