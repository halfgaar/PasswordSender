#ifndef PARSEDREQUEST_H
#define PARSEDREQUEST_H

#include <QObject>
#include <qfcgi/src/qfcgi.h>

class ParsedRequest : public QObject
{
    Q_OBJECT
public:
    explicit ParsedRequest(QFCgiRequest *parent = nullptr);

signals:

public slots:
};

#endif // PARSEDREQUEST_H
