#include "mynamedtempfile.h"
#include <QUuid>
#include <QFile>

MyNamedTempFile::MyNamedTempFile() :
    path(QString("/tmp/%1").arg(QUuid::createUuid().toString().replace('{', "").replace('}',"")))
{

}

MyNamedTempFile::~MyNamedTempFile()
{
    QFile f(path);

    if (f.exists())
        f.remove();
}
