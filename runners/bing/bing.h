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

#ifndef BING_H
#define BING_H

#include <Plasma/AbstractRunner>

#include <Plasma/RunnerContext>

#include <KIO/Job>

#include <KUrl>
#include <QDataStream>

class Bing : public Plasma::AbstractRunner {
    Q_OBJECT

public:
    Bing(QObject *parent, const QVariantList& args);
    ~Bing();

    void match(Plasma::RunnerContext &context);
    void run(const Plasma::RunnerContext &context, const Plasma::QueryMatch &match);

Q_SIGNALS:
    void matchMade(Plasma::RunnerContext *context);

private slots:
    void dataArrived(KIO::Job* job, const QByteArray& data);
    void startBingJob(Plasma::RunnerContext *context);
    void jobFinished(KJob* job);

private:
    void parseJson(const QByteArray& buffer);

    QDataStream buffer;
    KIO::StoredTransferJob *m_job;
};

Q_DECLARE_METATYPE(Plasma::RunnerContext*);

K_EXPORT_PLASMA_RUNNER(bing, Bing)

#endif
