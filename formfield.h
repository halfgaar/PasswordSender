#ifndef FORMFIELD_H
#define FORMFIELD_H

#include <QString>
#include <vmime/vmime.hpp>

class FormField
{
public:
    QString name;
    QString value;
    FormField();
    explicit FormField(const std::string &name, vmime::shared_ptr<const vmime::attachment> &attachment);
};

#endif // FORMFIELD_H
