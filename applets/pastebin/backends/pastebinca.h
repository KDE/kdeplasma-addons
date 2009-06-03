/***************************************************************************
 *   Copyright (C) 2007 by Thomas Georgiou <TAGeorgiou@gmail.com>          *
 *                         Artur Duque de Souza <morpheuz@gmail.com>       *
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

#ifndef PASTEBINCA_H
#define PASTEBINCA_H

#include "server.h"

#include <KConfigDialog>

#include <kio/global.h>
#include <kio/job.h>
#include <kjob.h>

class PastebinCAServer : public PastebinServer
{
    Q_OBJECT

public:
    PastebinCAServer(const KConfigGroup& config);
    virtual ~PastebinCAServer();

    virtual void post(const QString& content);

protected:
    const QByteArray m_apikey;

public slots:
    void finished(KJob *job);
    void readKIOData(KIO::Job *job, const QByteArray &data);

private:
    QByteArray _data;
};

#endif
