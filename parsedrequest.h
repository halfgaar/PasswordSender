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

#ifndef PARSEDREQUEST_H
#define PARSEDREQUEST_H

#include <QObject>
#include <QVector>
#include <qfcgi/src/qfcgi.h>
#include "uploadedfile.h"
#include "formfield.h"

#define RAN_BUF_SIZE 32

class ParsedRequest : public QObject
{
    Q_OBJECT
    char *ranBuf = nullptr;

public:
    std::vector<UploadedFile> files;
    QHash<QString,FormField> formFields;
    QHash<QString,QString> params;

    QString scriptURL;
    QString httpHost;
    QFCgiRequest *fcgiRequest = nullptr;

    QByteArray iv = QByteArray(32,0);
    QByteArray cipherKey = QByteArray(32,0);

    explicit ParsedRequest(QFCgiRequest *parent);
    ~ParsedRequest();
    void addFile(UploadedFile &uploaded_file);
    void addField(const FormField &formField);
    void requestDone(quint32 code);

public slots:
};

#endif // PARSEDREQUEST_H
