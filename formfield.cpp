#include "formfield.h"

#include <sstream>
#include <fstream>

FormField::FormField()
{

}

FormField::FormField(const std::string &name, vmime::shared_ptr<const vmime::attachment> &attachment) :
    name(QString::fromStdString(name))
{
    std::ostringstream oss;
    vmime::utility::outputStreamAdapter os2(oss);
    attachment->getData()->extract(os2);
    this->value = QString::fromStdString(oss.str());
}
