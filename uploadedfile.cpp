#include "uploadedfile.h"
#include <fstream>
#include <QTemporaryFile>


UploadedFile::UploadedFile(const std::string &name, vmime::shared_ptr<const vmime::attachment> &attachment) :
    name(QString::fromStdString(name))
{
    QTemporaryFile tempFile;
    tempFile.setAutoRemove(false);
    tempFile.open();
    pathToDataFile = tempFile.fileName();
    tempFile.close();

    std::ofstream dumpFile(pathToDataFile.toStdString(), std::ios_base::trunc);
    vmime::utility::outputStreamAdapter os2(dumpFile);
    attachment->getData()->extractRaw(os2);
    dumpFile.flush();
    dumpFile.close();
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
