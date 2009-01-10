/***************************************************************************
 *   Copyright (C) 2009 by Thomas Georgiou <TAGeorgiou@gmail.com>          *
 *                         Artur Duque de Souza <morpheuz@gmail.com>       *
 *                         Michał Ziąbkowski <mziab@o2.pl>                 *
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

#ifndef IMAGESHACK_H
#define IMAGESHACK_H

#include <kio/global.h>
#include <kio/job.h>

#include "server.h"

class ImageshackServer : public PastebinServer
{
    Q_OBJECT

public:
    ImageshackServer();
    ~ImageshackServer();

    void post(QString content);
    void finish();
    bool addPair(const QString& name, const QString& value);
    bool addFile(const QString& name,const QString& path);

signals:
    void postFinished(const QString &data);

protected:
    QByteArray m_buffer;
    QByteArray m_boundary;
    const QString m_server;

public slots:
    void readKIOData(KIO::Job *job, const QByteArray &data);

private:
    bool first;
};

#endif
