/*************************************************************************
 * Copyright 2009-2010 Sandro Andrade sandroandrade@kde.org              *
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
#include <QLinearGradient>

#include <KIcon>
#include <KColorUtils>
#include <KGlobalSettings>

TopDevelopersView::TopDevelopersView(KdeObservatory *kdeObservatory, const QHash<QString, bool> &topDevelopersViewProjects, const QMap<QString, KdeObservatory::Project> &projects, QGraphicsWidget *parent, Qt::WindowFlags wFlags)
: IViewProvider(kdeObservatory, parent, wFlags),
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
            createView(i18n("Top Developers - %1", i.key()), QString("Top Developers") + " - " + i.key());
    }
}

void TopDevelopersView::updateViews(const Plasma::DataEngine::Data &data)
{
    QString project = data["project"].toString();
    
    RankValueMap topProjectDevelopers = data[project].value<RankValueMap>();

    QGraphicsWidget *container = containerForView(QString("Top Developers") + " - " + project);
    if (!container)
        return;

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
        QColor color = QColor::fromHsv(qrand() % 256, 255, 190);
        QLinearGradient grad (0.0, 0.0, (qreal) widthFactor*rank, 0.0);
        grad.setColorAt(0, KColorUtils::darken(color, 0.8));
        grad.setColorAt(1, color);
        developerRect->setBrush(QBrush(grad));
        developerRect->setToolTip(i18np("%2 - %1 commit", "%2 - %1 commits", rank, developer));
        developerRect->setAcceptHoverEvents(true);
        developerRect->installSceneEventFilter(m_kdeObservatory);

        QGraphicsTextItem *commitsNumber = new QGraphicsTextItem(developer.split(' ')[0], developerRect);
        commitsNumber->setDefaultTextColor(QColor(255, 255, 255));
        commitsNumber->setFont(KGlobalSettings::smallestReadableFont());
        commitsNumber->setPos((qreal) 0, (qreal) ((developerRect->rect().height())/2)-(commitsNumber->boundingRect().height()/2));
        j++;
    }
}
