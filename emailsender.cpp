#include "emailsender.h"
#include "iostream"

EmailSender::EmailSender()
{
    vmimeSession = vmime::net::session::create();
    vmime::utility::url url("sendmail://localhost");
    transport = vmimeSession->getTransport(url);
}

void EmailSender::SendEmail(SubmittedSecret &secret)
{
    auto msg = vmime::make_shared<vmime::message>();

    auto header = msg->getHeader();
    auto body = msg->getBody();

    auto hfFactory = vmime::headerFieldFactory::getInstance();

    auto recipient = hfFactory->create(vmime::fields::TO);
    recipient->setValue(secret.recipient.toStdString());
    header->appendField(recipient);

    auto from = hfFactory->create(vmime::fields::FROM);
    from->setValue("Geborsteld Staal's Wachtwoordenkoerier <noreply@geborsteldstaal.nl>");
    header->appendField(from);

    auto subject = hfFactory->create(vmime::fields::SUBJECT);
    subject->setValue("U heeft een wachtwoord ontvangen");
    header->appendField(subject);

    QString bodyMsg = QString("Hallo,\n"
                              "\n"
                              "%1\n"
                              "\n"
                              "Groet,\n"
                              "\n"
                              "Geborsteld Staal").arg(secret.getLink());

    body->setContents(vmime::make_shared<vmime::stringContentHandler>(bodyMsg.toStdString()));

    transport->connect();
    transport->send(msg);
    transport->disconnect();
}
