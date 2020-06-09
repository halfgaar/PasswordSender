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

#ifndef UPLOADEDFILE_H
#define UPLOADEDFILE_H

#include <QString>
#include <string>
#include <vmime/vmime.hpp>
#include "outputstreamqiodeviceadapter.h"

class UploadedFile
{
public:
    QString name;
    QString pathToDataFile;

    explicit UploadedFile(const std::string &name, vmime::shared_ptr<const vmime::attachment> &attachment, const QByteArray &iv, const QByteArray &cipherKey);
    UploadedFile(const UploadedFile &other) = delete;
    UploadedFile(UploadedFile &&other);
    ~UploadedFile();

    /**
     * @brief renameAndrelease rename the file, thereby it will automatically no longer removed by this class' destructor.
     * @param newPath
     */
    void renameAndrelease(const QString &newPath);
};

#endif // UPLOADEDFILE_H
