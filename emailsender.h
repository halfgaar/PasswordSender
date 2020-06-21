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

#ifndef EMAILSENDER_H
#define EMAILSENDER_H

#include <vmime/vmime.hpp>
#include <QString>
#include <submittedsecret.h>

class EmailSender
{
    vmime::shared_ptr<vmime::net::session> vmimeSession;
    vmime::shared_ptr<vmime::net::transport> transport;
    QString from;
    QString subject;
    QString templatePath;
public:
    EmailSender(const QString &from, const QString &subject, const QString &templateDir);
    void SendEmail(SubmittedSecret &secret);
};

#endif // EMAILSENDER_H
