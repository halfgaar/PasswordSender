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

#ifndef INPUTSTATEANDREQUEST_H
#define INPUTSTATEANDREQUEST_H

#include <QObject>
#include <QIODevice>
#include <QTimer>
#include "qfcgi/src/qfcgi.h"
#include "parsedrequest.h"

#include "uploadedfile.h"
#include "formfield.h"

// TODO: hide privates better.
class RequestDownloader : public QObject
{
    Q_OBJECT

    void parseRequest();

private slots:
    void onTimeout();

public:

    QIODevice *input = nullptr;
    QFCgiRequest *request = nullptr;
    std::unique_ptr<QByteArray> requestData;
    int rI = 0;
    int parsed = false;
    QString contentType;
    QTimer timeoutTimer;

    explicit RequestDownloader(QIODevice *input, QFCgiRequest *request, int contentLength);

    void readAvailableData();

signals:
    void requestParsed(ParsedRequest *parsedRequest);

};

#endif // INPUTSTATEANDREQUEST_H
