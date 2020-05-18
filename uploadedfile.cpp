#include "uploadedfile.h"
#include <fstream>
#include <QTemporaryFile>
#include <QFile>


UploadedFile::UploadedFile(const std::string &name, vmime::shared_ptr<const vmime::attachment> &attachment) :
    name(QString::fromStdString(name))
{
    QTemporaryFile tempFile;
    tempFile.setAutoRemove(false);
    tempFile.open();
    pathToDataFile = tempFile.fileName();
    tempFile.close();

    QFile outputFile(pathToDataFile);
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
