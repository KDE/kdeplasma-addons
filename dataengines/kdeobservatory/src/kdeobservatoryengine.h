/*************************************************************************
 * Copyright 2010 Sandro Andrade sandroandrade@kde.org                   *
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

#ifndef KDEOBSERVATORYENGINE_HEADER
#define KDEOBSERVATORYENGINE_HEADER

#include <Plasma/DataEngine>

typedef QMultiMap<int, QString> RankValueMap;
typedef QList< QPair<QString, int> > DateCommitList;

//            FileType      TestName      FileName       Error
typedef QMap< QString, QMap<QString, QMap<QString, QList<QString> > > > KrazyReportMap;

class KdeObservatoryEngine : public Plasma::DataEngine
{
    Q_OBJECT
public:
    KdeObservatoryEngine(QObject *parent, const QVariantList &args);

    void init();

    bool sourceRequestEvent (const QString &source);
    Plasma::Service *serviceForSource(const QString &source);

    friend class KdeObservatoryService;

Q_SIGNALS:
    void engineReady();

private:
    QHash< QString, QPair <QMap<QString, QVariant>, QVariant> > m_dataCache;
};

Q_DECLARE_METATYPE(RankValueMap)
Q_DECLARE_METATYPE(DateCommitList)
Q_DECLARE_METATYPE(KrazyReportMap)

#endif
