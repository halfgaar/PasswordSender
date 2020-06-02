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

    getrandom(ranBuf, RAN_BUF_SIZE, 0);
    iv = QByteArray(ranBuf, RAN_BUF_SIZE);

    getrandom(ranBuf, RAN_BUF_SIZE, 0);
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
