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

#ifndef DUCKDUCKGO_H
#define DUCKDUCKGO_H

#include <Plasma/AbstractRunner>

#include <Plasma/RunnerContext>

#include <KIO/Job>

#include <KUrl>

class QXmlStreamReader;

class DuckDuckGo : public Plasma::AbstractRunner {
    Q_OBJECT

public:
    DuckDuckGo(QObject *parent, const QVariantList& args);
    ~DuckDuckGo();

    void match(Plasma::RunnerContext &context);
    void run(const Plasma::RunnerContext &context, const Plasma::QueryMatch &match);

Q_SIGNALS:
    void matchMade(Plasma::RunnerContext *context);

private slots:
    void dataArrived(KIO::Job* job, const QByteArray& data);
    void startDuckDuckGoJob(Plasma::RunnerContext *context);

private:
    void parseXML(QByteArray);
    void parseVideo(QXmlStreamReader& xml);

    QByteArray buffer;

    Plasma::RunnerContext *m_context;
};

Q_DECLARE_METATYPE(Plasma::RunnerContext*);

K_EXPORT_PLASMA_RUNNER(duckduckgo, DuckDuckGo)

#endif
