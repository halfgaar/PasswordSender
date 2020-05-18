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

    this->scriptURL = this->params["SCRIPT_URL"]; // I need this for sure, so I want a variable

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
