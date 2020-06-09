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

#include "outputstreamqiodeviceadapter.h"

void OutputStreamQIODeviceAdapter::flush()
{
    this->file.flush();
}

void OutputStreamQIODeviceAdapter::writeImpl(const vmime::byte_t * const data, const size_t count)
{
    const char *charData = reinterpret_cast<const char *>(data);
    if (file.write(charData, static_cast<qint64>(count)) < 0)
    {
        throw std::runtime_error(QString("Writing to %1 failed").arg(this->file.fileName()).toStdString());
    }
}

OutputStreamQIODeviceAdapter::OutputStreamQIODeviceAdapter(QFile &qFile, std::ostringstream dummy) :
    vmime::utility::outputStreamAdapter(dummy),
    file(qFile)
{

}
