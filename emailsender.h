#ifndef EMAILSENDER_H
#define EMAILSENDER_H

#include <vmime/vmime.hpp>
#include <QString>
#include <submittedsecret.h>

class EmailSender
{
    vmime::shared_ptr<vmime::net::session> vmimeSession;
    vmime::shared_ptr<vmime::net::transport> transport;
public:
    EmailSender();
    void SendEmail(SubmittedSecret &secret);
};

#endif // EMAILSENDER_H
