#ifndef USERERROR_H
#define USERERROR_H

#include <stdexcept>
#include <QString>

class UserError : public std::runtime_error
{
public:
    const int httpCode;

    UserError(std::string msg, int httpCode=200);
    UserError(QString &msg,int httpCode=200);
};

#endif // USERERROR_H
