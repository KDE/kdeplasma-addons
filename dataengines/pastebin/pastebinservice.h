/*
 *   Copyright 2009 Artur Duque de Souza <asouza@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2 or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef PASTEBIN_SERVICE_H
#define PASTEBIN_SERVICE_H

#include "pastebinengine.h"
#include "backends/backends.h"

#include <Plasma/Service>


class QNetworkAccessManager;
class QNetworkReply;

namespace Plasma {
    class ServiceJob;
}

class PastebinService : public Plasma::Service
{
    Q_OBJECT

public:
    enum textServers { PASTEBINCA, PASTEBINCOM };
    enum imageServers { IMAGEBINCA, IMAGESHACK, SIMPLESTIMAGEHOSTING, IMGUR };

    PastebinService(PastebinEngine *engine);
    Plasma::ServiceJob *createJob(const QString &operation,
                                  QMap<QString, QVariant> &parameters);

protected:
    void postText(QMap<QString, QVariant> &parameters);
    void postImage(QMap<QString, QVariant> &parameters);

protected slots:
    void processTinyUrl(QNetworkReply *reply);
    void showResults(const QString &url);
    void showErrors(const QString &url = "");

private:
    PastebinEngine *m_engine;

    PastebinServer *m_textServer;
    PastebinServer *m_imageServer;

    // to handle request to tinyUrl
    QNetworkAccessManager *manager;
};

#endif // PASTEBIN_SERVICE
