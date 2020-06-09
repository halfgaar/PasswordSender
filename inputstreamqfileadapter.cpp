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

#include "inputstreamqfileadapter.h"

InputStreamQFileAdapter::InputStreamQFileAdapter(QFile &qFile, std::istringstream dummy) :
    vmime::utility::inputStreamAdapter(dummy),
    file(qFile)
{

}

bool InputStreamQFileAdapter::eof() const
{
    return file.atEnd();
}

void InputStreamQFileAdapter::reset()
{
    if (!file.reset())
    {
        throw std::runtime_error(QString("Reset %1 failed").arg(this->file.fileName()).toStdString());
    }
}

size_t InputStreamQFileAdapter::read(vmime::byte_t * const data, const size_t count)
{
    char * const charData = reinterpret_cast<char * const>(data);
    qint64 read = this->file.read(charData, static_cast<qint64>(count));
    if (read < 0)
    {
        throw std::runtime_error(QString("Reading from %1 failed").arg(this->file.fileName()).toStdString());
    }

    return static_cast<size_t>(read);
}

size_t InputStreamQFileAdapter::skip(const size_t count)
{
    qint64 newPos = file.pos() + static_cast<qint64>(count);
    if (!this->file.seek(newPos))
    {
        throw std::runtime_error(QString("Seek/skip error in %1").arg(this->file.fileName()).toStdString());
    }

    return count;
}

size_t InputStreamQFileAdapter::getPosition() const
{
    return static_cast<size_t>(this->file.pos());
}

void InputStreamQFileAdapter::seek(const size_t pos)
{
    if (!this->file.seek( static_cast<qint64>(pos) ))
    {
        throw std::runtime_error(QString("Seek error in %1").arg(this->file.fileName()).toStdString());
    }
}
