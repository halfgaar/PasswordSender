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
