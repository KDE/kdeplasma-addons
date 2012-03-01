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

#include <QNetworkAccessManager>
#include <Plasma/AbstractRunner>
#include <Plasma/RunnerContext>

#include <KUrl>
#include <QByteArray>

class Bing : public Plasma::AbstractRunner {
    Q_OBJECT

public:
    Bing(QObject *parent, const QVariantList& args);
    ~Bing();

    void match(Plasma::RunnerContext &context);
    void run(const Plasma::RunnerContext &context, const Plasma::QueryMatch &match);

Q_SIGNALS:
    void matchMade(Plasma::RunnerContext *context);

private:
    void parseJson(const QByteArray& data, Plasma::RunnerContext &context);

    QNetworkAccessManager *m_thumbnailDownloader;
};

K_EXPORT_PLASMA_RUNNER(bing, Bing)

#endif
