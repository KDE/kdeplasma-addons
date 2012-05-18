/*
 *   Copyright 2008 Aaron Seigo <aseigo@kde.org>
 *   Copyright (C) 2009 Ryan P. Bitanga <ryan.bitanga@gmail.com>
 *   Copyright 2012 Sebastian Kügler <sebas@kde.org>
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

#ifndef TWEETJOB_H
#define TWEETJOB_H


#include <KUrl>

#include <Plasma/DataContainer>
#include <Plasma/Service>
#include <Plasma/ServiceJob>

// forward declarations
//class QXmlStreamReader;

class KJob;

namespace KIO
{
    class Job;
} // namespace KIO

namespace QCA
{
    class Initializer;
}

class TimelineSource;
class ImageSource;

class TweetJob : public Plasma::ServiceJob
{
    Q_OBJECT

public:
    TweetJob(TimelineSource *source, const QString &operation, const QMap<QString, QVariant> &parameters, QObject *parent = 0);
    void start();

Q_SIGNALS:
    void userData(const QByteArray &data);

private slots:
    void result(KJob *job);
    void recv(KIO::Job*, const QByteArray& data);

private:
    KUrl m_url;
    QByteArray m_data;
    QMap<QString, QVariant> m_parameters;
    TimelineSource *m_source;
    QString m_operation;
};

#endif
