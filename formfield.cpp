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

#include "formfield.h"

#include <sstream>
#include <fstream>

FormField::FormField()
{

}

FormField::FormField(const std::string &name, vmime::shared_ptr<const vmime::attachment> &attachment) :
    name(QString::fromStdString(name))
{
    std::ostringstream oss;
    vmime::utility::outputStreamAdapter os2(oss);
    attachment->getData()->extract(os2);
    this->value = QString::fromStdString(oss.str());
}
