/***************************************************************************
 *   Copyright (C) 2008 by Artur Duque de Souza <morpheuz@gmail.com>       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

#ifndef PASTEBINCOM_H
#define PASTEBINCOM_H

#include "server.h"

#include <KConfigDialog>

#include <kio/global.h>
#include <kio/job.h>

#define PASTEBINCOM_SERVER "http://pastebin.com"

class PastebinCOMServer : public PastebinServer
{
    Q_OBJECT

public:
    PastebinCOMServer(const QString &server = PASTEBINCOM_SERVER);
    virtual ~PastebinCOMServer();

    virtual void post(const QString& content);

private Q_SLOTS:
    void data(KIO::Job* job, const QByteArray &data);

private:
    QString m_resultingUrl;
};

#endif
