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
