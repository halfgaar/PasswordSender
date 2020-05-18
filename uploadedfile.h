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
