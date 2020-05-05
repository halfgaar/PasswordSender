#include "parsedrequest.h"

ParsedRequest::ParsedRequest(QFCgiRequest *parent) : QObject(parent)
{
    // TODO: store fcgi params
}

void ParsedRequest::addFile(UploadedFile &uploaded_file)
{
    this->files.push_back(std::move(uploaded_file));
}

void ParsedRequest::addField(const FormField &formField)
{
    this->formFields.append(formField);
}
