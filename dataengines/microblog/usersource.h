/*
 *   Copyright 2008 Aaron Seigo <aseigo@kde.org>
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

#ifndef USERSOURCE_H
#define USERSOURCE_H

#include <QList>
#include <QPair>
#include <QXmlStreamReader>

#include <KUrl>

#include <Plasma/DataContainer>

// forward declarations
class KJob;
class KImageCache;

namespace KIO
{
    class Job;
} // namespace KIO

class UserSourcePrivate;

class UserSource : public Plasma::DataContainer
{
Q_OBJECT

public:
    UserSource(const QString &who,  const QString &serviceBaseUrl, QObject* parent);
    ~UserSource();

    void loadUserInfo(const QString &who, const QString &serviceBaseUrl);
    void parseJson(const QVariant &data);

public Q_SLOTS:
    void parse(const QByteArray &jsonData);

Q_SIGNALS:
    void dataChanged();
    void loadImage(const QString &who, const KUrl&);

private Q_SLOTS:
    void recv(KIO::Job*, const QByteArray& data);
    void result(KJob*);

private:
    void parseJsonStatus(const QVariant &data);
    void parse(QXmlStreamReader &reader);
    void readUser(QXmlStreamReader &reader);

    UserSourcePrivate* d;

    QString m_user;
    QString m_serviceBaseUrl;
    QByteArray m_xml;
    QHash<KJob *, QString> m_jobs;
    QHash<KJob *, QByteArray> m_jobData;
    int m_runningJobs;
    QList<QPair<QString, KUrl> > m_queuedJobs;
    Plasma::DataEngine::Data m_cachedData;
    QString m_currentUrl;
};

#endif

