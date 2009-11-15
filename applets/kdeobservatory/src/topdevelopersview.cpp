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

#include "topdevelopersview.h"

#include <QPen>

#include <KIcon>
#include <KGlobalSettings>

#include "kdeobservatorydatabase.h"

TopDevelopersView::TopDevelopersView(const QHash<QString, bool> &topDevelopersViewProjects, const QMap<QString, KdeObservatory::Project> &projects, QRectF rect, QGraphicsWidget *parent, Qt::WindowFlags wFlags)
: IViewProvider(rect, parent, wFlags),
  m_topDevelopersViewProjects(topDevelopersViewProjects),
  m_projects(projects)
{
}

TopDevelopersView::~TopDevelopersView()
{
}

void TopDevelopersView::updateViews()
{
    deleteViews();

    QMap< QString, QMultiMap<int, QString> > topDevelopers;
    QHashIterator<QString, bool> i1(m_topDevelopersViewProjects);
    while (i1.hasNext())
    {
        i1.next();
        if (i1.value())
            topDevelopers.insert(i1.key(), KdeObservatoryDatabase::self()->developersByProject(m_projects[i1.key()].commitSubject));
    }

    QMapIterator< QString, QMultiMap<int, QString> > i2(topDevelopers);
    while (i2.hasNext())
    {
        i2.next();

        QString project = i2.key();
        const QMultiMap<int, QString> &projectDevelopers = i2.value();

        QMapIterator<int, QString> i3(projectDevelopers);
        i3.toBack();

        QGraphicsWidget *container = createView(i18n("Top Developers") + " - " + project);

        int maxRank = 0;
        qreal width = container->geometry().width();
        qreal step = qMax(container->geometry().height() / projectDevelopers.size(), (qreal) 22);

        int j = 0;
        while (i3.hasPrevious())
        {
            i3.previous();
            QString developer = i3.value();
            int rank = i3.key();
            if (j == 0)
                maxRank = rank;

            qreal widthFactor = (width-24)/maxRank;
            qreal yItem = (j*step)+2;

            QGraphicsRectItem *developerRect = new QGraphicsRectItem(0, 0, (qreal) widthFactor*rank, (qreal) step-4, container);
            developerRect->setPos(0, yItem);
            developerRect->setPen(QPen(QColor(0, 0, 0)));
            developerRect->setBrush(QBrush(QColor::fromHsv(qrand() % 256, 255, 190), Qt::SolidPattern));
            developerRect->setToolTip(developer + " - " + QString::number(rank) + ' ' + i18n("commits"));

            QGraphicsTextItem *commitsNumber = new QGraphicsTextItem(developer.split(' ')[0], developerRect);
            commitsNumber->setDefaultTextColor(QColor(255, 255, 255));
            commitsNumber->setFont(KGlobalSettings::smallestReadableFont());
            commitsNumber->setPos((qreal) 0, (qreal) ((developerRect->rect().height())/2)-(commitsNumber->boundingRect().height()/2));
            j++;
        }
    }
}
