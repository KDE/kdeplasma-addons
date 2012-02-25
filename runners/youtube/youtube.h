/******************************************************************************
 *  Copyright (C) 2012 by Shaun Reich <sreich@kde.org>                        *
 *                                                                            *
 *  This library is free software; you can redistribute it and/or modify      *
 *  it under the terms of the GNU Lesser General Public License as published  *
 *  by the Free Software Foundation; either version 2 of the License or (at   *
 *  your option) any later version.                                           *
 *                                                                            *
 *  This library is distributed in the hope that it will be useful,           *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU         *
 *  Library General Public License for more details.                          *
 *                                                                            *
 *  You should have received a copy of the GNU Lesser General Public License  *
 *  along with this library; see the file COPYING.LIB.                        *
 *  If not, see <http://www.gnu.org/licenses/>.                               *
 *****************************************************************************/

#ifndef YOUTUBE_H
#define YOUTUBE_H

#include <Plasma/AbstractRunner>

#include <Plasma/RunnerContext>

#include <KIO/Job>

#include <KUrl>


class QXmlStreamReader;

class YouTube : public Plasma::AbstractRunner {
    Q_OBJECT

public:
    YouTube(QObject *parent, const QVariantList& args);
    ~YouTube();

    void match(Plasma::RunnerContext &context);
    void run(const Plasma::RunnerContext &context, const Plasma::QueryMatch &match);

Q_SIGNALS:
    void matchMade(Plasma::RunnerContext *context);

private slots:
    void dataArrived(KIO::Job* job, const QByteArray& data, Plasma::RunnerContext* context);
    void startYouTubeJob(Plasma::RunnerContext *context);

private:
    void parseXML(QByteArray);
    void parseVideo(QXmlStreamReader& xml);

    QByteArray buffer;

    Plasma::RunnerContext *m_context;
};

class TubeJob : public QObject
{
    Q_OBJECT

public:
    TubeJob(const KUrl& url, KIO::LoadType type, KIO::JobFlag flags, Plasma::RunnerContext *context);

    void start();

Q_SIGNALS:
    void dataReceived(KIO::Job* job, QByteArray data, Plasma::RunnerContext *context);

private Q_SLOTS:
    void onData(KIO::Job* job, QByteArray data);

private:
    Plasma::RunnerContext *m_context;

    KIO::TransferJob *m_job;
};

Q_DECLARE_METATYPE(Plasma::RunnerContext*);

K_EXPORT_PLASMA_RUNNER(youtube, YouTube)

#endif
