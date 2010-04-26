/*************************************************************************
 * Copyright 2009 Sandro Andrade sandroandrade@kde.org                   *
 *                                                                       *
 * This program is free software; you can redistribute it and/or         *
 * modify it under the terms of the GNU General Public License as        *
 * published by the Free Software Foundation; either version 2 of        *
 * the License or (at your option) version 3 or any later version        *
 * accepted by the membership of KDE e.V. (or its successor approved     *
 * by the membership of KDE e.V.), which shall act as a proxy            *
 * defined in Section 14 of version 3 of the license.                    *
 *                                                                       *
 * This program is distributed in the hope that it will be useful,       *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 * GNU General Public License for more details.                          *
 *                                                                       *
 * You should have received a copy of the GNU General Public License     *
 * along with this program.  If not, see <http://www.gnu.org/licenses/>. *
 * ***********************************************************************/

#ifndef COMMITCOLLECTOR_HEADER
#define COMMITCOLLECTOR_HEADER

#include "icollector.h"

#include <QStack>

class CommitCollector : public ICollector
{
    Q_OBJECT
public:
    CommitCollector(QObject *parent = 0);
    virtual ~CommitCollector();

    void setExtent (int extent);
    int extent() const;

    void setCommitsRead(int commitsRead);
    int commitsRead() const;

    void setLastArchiveRead(QString lastArchiveRead);
    QString lastArchiveRead() const;

    void setFullUpdate(bool fullUpdate);
    bool fullUpdate() const;

    virtual void run();

    typedef struct
    {
        QString date;
        QString subject;
        QString developer;
    } Commit;

    QNetworkReply *runServletOperation(const QString &operation, const QStringList &args);

protected Q_SLOTS:
    virtual void replyFinished(QNetworkReply *reply);

private:
    bool m_fullUpdate;
    int  m_commitsRead;
    int  m_commitsToBeRead;
    int  m_initialCommitsToBeRead;

    int m_extent;
    int m_page;
    long long m_stopCollectingDay;
    long long m_lastDay;
    int m_dayCounter;

    QStack<Commit> m_commits;
    QString m_archiveName;
    QString m_lastArchiveRead;
    QString m_initialArchiveName;
};

#endif
