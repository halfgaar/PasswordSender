#ifndef INPUTSTREAMQFILEADAPTER_H
#define INPUTSTREAMQFILEADAPTER_H

#include <vmime/vmime.hpp>
#include <QFile>
#include <iostream>

class InputStreamQFileAdapter : vmime::utility::inputStreamAdapter
{
    QFile &file;
public:
    InputStreamQFileAdapter(QFile &qFile, std::istringstream dummy = std::istringstream());

    bool eof() const override;
    void reset() override;
    size_t read(vmime::byte_t* const data, const size_t count) override;
    size_t skip(const size_t count) override;
    size_t getPosition() const override;
    void seek(const size_t pos) override;
};

#endif // INPUTSTREAMQFILEADAPTER_H
