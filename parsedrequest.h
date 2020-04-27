#ifndef PARSEDREQUEST_H
#define PARSEDREQUEST_H

#include <QObject>

class ParsedRequest : public QObject
{
    Q_OBJECT
public:
    explicit ParsedRequest(QObject *parent = nullptr);

signals:

public slots:
};

#endif // PARSEDREQUEST_H
