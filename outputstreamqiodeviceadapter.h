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

#ifndef OUTPUTSTREAMQIODEVICEADAPTER_H
#define OUTPUTSTREAMQIODEVICEADAPTER_H

#include "vmime/vmime.hpp"
#include <ostream>
#include <QFile>

class OutputStreamQIODeviceAdapter : public vmime::utility::outputStreamAdapter
{
    QFile &file;

    void flush() override;
protected:
    void writeImpl(const vmime::byte_t* const data, const size_t count) override;
public:
    OutputStreamQIODeviceAdapter(QFile &qFile, std::ostringstream dummy = std::ostringstream());
};

#endif // OUTPUTSTREAMQIODEVICEADAPTER_H
