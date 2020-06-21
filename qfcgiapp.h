/**
  * This file is part of PasswordSender.
  *
  * PasswordSender is free software: you can redistribute it and/or modify
  * it under the terms of the GNU General Public License as published by
  * the Free Software Foundation, either version 3 of the License, or
  * (at your option) any later version.

  * PasswordSender is distributed in the hope that it will be useful,
  * but WITHOUT ANY WARRANTY; without even the implied warranty of
  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  * GNU General Public License for more details.
  *
  * You should have received a copy of the GNU General Public License
  * along with PasswordSender. If not, see <https://www.gnu.org/licenses/>.
  *
  * Copyright 2020 Wiebe Cazemier <wiebe@halfgaar.net>
  */

#ifndef QFCGIAPP_H
#define QFCGIAPP_H

#include <QObject>
#include <QCoreApplication>
#include "qfcgi/src/qfcgi.h"
#include "requestdownloader.h"
#include "requestuploader.h"
#include "submittedsecret.h"
#include "emailsender.h"
#include <QHash>
#include "usererror.h"
#include "QTimer"

class QFcgiApp : public QCoreApplication
{
    QFCgi *fcgi = nullptr;
    QHash<QIODevice*, RequestDownloader*> requests;
    QHash<QString, SubmittedSecret_p> submittedSecrets;
    QScopedPointer<EmailSender> emailSender;
    QTimer cleanupTimer;
    QString templateDir;
    bool initialized = false;

    void renderReponse(const QString &templateFileName, const int httpCode, QIODevice *out, const QHash<QString, QString> &templateVariables);

public:
    QFcgiApp(int argc, char *argv[]);
    ~QFcgiApp();
    bool isInitialized() { return initialized; }

private slots:
    void onNewRequest(QFCgiRequest *request);
    void onReadyRead();
    void onUploadDone();
    void requestParsed(ParsedRequest *parsedRequest);
    void onConnectionClose();
    void onCleanupTimerElapsed();
};

#endif // QFCGIAPP_H
