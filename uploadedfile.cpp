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

#include "uploadedfile.h"
#include <fstream>
#include <QTemporaryFile>
#include <QFile>
#include <qfileencrypted.h>


UploadedFile::UploadedFile(const std::string &name, vmime::shared_ptr<const vmime::attachment> &attachment, const QByteArray &iv, const QByteArray &cipherKey) :
    name(QString::fromStdString(name))
{
    QTemporaryFile tempFile;
    tempFile.setAutoRemove(false);
    tempFile.open();
    pathToDataFile = tempFile.fileName();
    tempFile.close();

    QFileEncrypted outputFile(pathToDataFile, iv, cipherKey);
    if (!outputFile.open(QFile::WriteOnly))
        throw std::runtime_error(QString("Cannot open %1").arg(pathToDataFile).toStdString());
    OutputStreamQIODeviceAdapter os3(outputFile);
    attachment->getData()->extractRaw(os3);
    outputFile.flush();
    outputFile.close();
}

UploadedFile::UploadedFile(UploadedFile &&other)
{
    name = other.name;
    pathToDataFile = other.pathToDataFile;

    other.pathToDataFile.clear();
}

UploadedFile::~UploadedFile()
{
    if (!this->pathToDataFile.isEmpty() && QFile::exists(this->pathToDataFile))
    {
        QFile::remove(this->pathToDataFile);
    }
}

void UploadedFile::renameAndrelease(const QString &newPath)
{
    QFile::rename(this->pathToDataFile, newPath);
}
