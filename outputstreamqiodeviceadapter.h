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
