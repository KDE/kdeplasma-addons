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

TopDevelopersView::TopDevelopersView(const QHash<QString, bool> &topDevelopersViewProjects, const QMap<QString, KdeObservatory::Project> &projects, QGraphicsWidget *parent, Qt::WindowFlags wFlags)
: IViewProvider(parent, wFlags),
  m_topDevelopersViewProjects(topDevelopersViewProjects),
  m_projects(projects)
{
}

TopDevelopersView::~TopDevelopersView()
{
}

void TopDevelopersView::createViews()
{
    deleteViews();
    QHashIterator<QString, bool> i(m_topDevelopersViewProjects);
    while (i.hasNext())
    {
        i.next();
        if (i.value())
            createView(i18n("Top Developers") + " - " + i.key());
    }
}

void TopDevelopersView::updateViews(const Plasma::DataEngine::Data &data)
{
    QString project = data["project"].toString();
    if (project.isEmpty())
        return;
    
    RankValueMap topProjectDevelopers = data["topProjectDevelopers"].value<RankValueMap>();

    KdeObservatory *kdeObservatory = dynamic_cast<KdeObservatory *>(m_parent->parentItem()->parentItem());

    QGraphicsWidget *container = containerForView(i18n("Top Developers") + " - " + project);

    int maxRank = 0;
    qreal width = container->geometry().width();
    qreal step = 22;

    RankValueMapIterator i(topProjectDevelopers);
    i.toBack();
    int j = 0;
    while (i.hasPrevious())
    {
        i.previous();
        QString developer = i.value();
        int rank = i.key();
        if (j == 0)
            maxRank = rank;

        qreal widthFactor = (width-24)/maxRank;
        qreal yItem = (j*step)+2;

        if (yItem + step-4 > container->geometry().height())
            break;

        QGraphicsRectItem *developerRect = new QGraphicsRectItem(0, 0, (qreal) widthFactor*rank, (qreal) step-4, container);
        developerRect->setPos(0, yItem);
        developerRect->setPen(QPen(QColor(0, 0, 0)));
        developerRect->setBrush(QBrush(QColor::fromHsv(qrand() % 256, 255, 190), Qt::SolidPattern));
        developerRect->setToolTip(developer + " - " + QString::number(rank) + ' ' + i18n("commits"));
        developerRect->setAcceptHoverEvents(true);
        developerRect->installSceneEventFilter(kdeObservatory);

        QGraphicsTextItem *commitsNumber = new QGraphicsTextItem(developer.split(' ')[0], developerRect);
        commitsNumber->setDefaultTextColor(QColor(255, 255, 255));
        commitsNumber->setFont(KGlobalSettings::smallestReadableFont());
        commitsNumber->setPos((qreal) 0, (qreal) ((developerRect->rect().height())/2)-(commitsNumber->boundingRect().height()/2));
        j++;
    }
}
