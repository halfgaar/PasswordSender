#include "usererror.h"

UserError::UserError(std::string msg, int httpCode) :
    std::runtime_error(msg),
    httpCode(httpCode)
{

}

UserError::UserError(QString &msg, int httpCode) :
    std::runtime_error(msg.toStdString()),
    httpCode(httpCode)
{

}
