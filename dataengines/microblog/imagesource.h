/*
 *   Copyright 2008 Aaron Seigo <aseigo@kde.org>
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

#ifndef IMAGESOURCE_H
#define IMAGESOURCE_H

#include <QList>
#include <QPair>
#include <QXmlDefaultHandler>

#include <KUrl>

#include <Plasma/DataContainer>

// forward declarations
class KJob;

namespace KIO
{
    class Job;
} // namespace KIO

class ImageSource : public Plasma::DataContainer
{
Q_OBJECT

public:
    ImageSource(QObject* parent);
    ~ImageSource();

    void loadImage(const QString &who, const KUrl &url);

private slots:
    void recv(KIO::Job*, const QByteArray& data);
    void result(KJob*);

private:
    QHash<KJob *, QString> m_jobs;
    QHash<KJob *, QByteArray> m_jobData;
    int m_runningJobs;
    QList<QPair<QString, KUrl> > m_queuedJobs;
};

#endif

