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

#include "parsedrequest.h"
#include "algorithm"
#include <iostream>

#include <sys/random.h>

ParsedRequest::ParsedRequest(QFCgiRequest *parent) : QObject(parent)
{
    this->fcgiRequest = parent;
    for(const QString &s : parent->getParams())
    {
        this->params[s] = parent->getParam(s);
    }

    /* There's a difference between Apache's fcgi params and Nginx's. Apache passes, for instance:
     *
     *   SCRIPT_URL = /passwordsender/upload?querystringarg=1 (although we don't use query strings.
     *   SCRIPT_URI = https://passwords.mydomain.nl/passwordsender/upload?querystringarg=1
     *   HTTP_HOST = passwords.mydomain.nl
     *   [nothing for request method?]
     *
     * Nginx passes (I think, I didn't test):
     *
     *   REQUEST_URI = /passwordsender/upload?querystringarg=1
     *   SERVER_NAME = passwords.mydomain.nl
     *   REQUEST_METHOD = GET/POST/PUT/DELETE?
     *
     * TODO: detect which ones and unify.
     *
     * Anyway, we store the important ones in variables, because I want easy access.
     */
    this->scriptURL = this->params["SCRIPT_URL"];
    this->httpHost = this->params["HTTP_HOST"];

    ranBuf = new char[RAN_BUF_SIZE];

    if (getrandom(ranBuf, RAN_BUF_SIZE, 0) != RAN_BUF_SIZE)
        throw std::runtime_error("Not enough random entroy?");
    iv = QByteArray(ranBuf, RAN_BUF_SIZE);

    if (getrandom(ranBuf, RAN_BUF_SIZE, 0) != RAN_BUF_SIZE)
        throw std::runtime_error("Not enough random entroy?");
    cipherKey = QByteArray(ranBuf, RAN_BUF_SIZE);
}

ParsedRequest::~ParsedRequest()
{
    delete[] ranBuf;
}

void ParsedRequest::addFile(UploadedFile &uploaded_file)
{
    this->files.push_back(std::move(uploaded_file));
}

void ParsedRequest::addField(const FormField &formField)
{
    this->formFields[formField.name] = formField;
}

void ParsedRequest::requestDone(quint32 code)
{
    this->fcgiRequest->endRequest(code);
}
