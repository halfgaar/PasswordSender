#include "parsedrequest.h"
#include "algorithm"
#include <iostream>

ParsedRequest::ParsedRequest(QFCgiRequest *parent) : QObject(parent)
{
    this->fcgiRequest = parent;
    for(const QString &s : parent->getParams())
    {
        this->params[s] = parent->getParam(s);
    }

    this->scriptURL = this->params["SCRIPT_URL"]; // I need this for sure, so I want a variable
}

void ParsedRequest::addFile(UploadedFile &uploaded_file)
{
    this->files.push_back(std::move(uploaded_file));
}

void ParsedRequest::addField(const FormField &formField)
{
    this->formFields[formField.name] = formField;
}
