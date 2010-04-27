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

#include "topactiveprojectsview.h"

#include <QPen>
#include <QFontMetrics>

#include <KIcon>
#include <KGlobalSettings>

TopActiveProjectsView::TopActiveProjectsView(const QHash<QString, bool> &topActiveProjectsViewProjects, const QMap<QString, KdeObservatory::Project> &projects, QGraphicsWidget *parent, Qt::WindowFlags wFlags)
: IViewProvider(parent, wFlags),
  m_parent(parent),
  m_topActiveProjectsViewProjects(topActiveProjectsViewProjects),
  m_projects(projects)
{
}

TopActiveProjectsView::~TopActiveProjectsView()
{
}

void TopActiveProjectsView::createViews()
{
    deleteViews();
    createView(i18n("Top Active Projects"));
}

void TopActiveProjectsView::updateViews(const Plasma::DataEngine::Data &data)
{
    QMultiMap<int, QString> topActiveProjects = data["topActiveProjects"].value< QMultiMap<int, QString> >();

    KdeObservatory *kdeObservatory = dynamic_cast<KdeObservatory *>(m_parent->parentItem()->parentItem());

    QGraphicsWidget *container = containerForView(i18n("Top Active Projects"));

    int maxRank = 0;
    qreal width = container->geometry().width();
    qreal step = 22;

    QMapIterator<int, QString> i2(topActiveProjects);
    i2.toBack();
    int j = 0;
    while (i2.hasPrevious())
    {
        i2.previous();
        QString project = i2.value();
        if (m_topActiveProjectsViewProjects[project])
        {
            int rank = i2.key();
            if (j == 0)
                maxRank = rank;

            qreal widthFactor = (width-24)/maxRank;
            qreal yItem = (j*step)+2;

            if (yItem + step-4 > container->geometry().height())
                break;

            QGraphicsRectItem *projectRect = new QGraphicsRectItem(0, 0, (qreal) widthFactor*rank, (qreal) step-4, container);
            projectRect->setPos(0, yItem);
            projectRect->setPen(QPen(QColor(0, 0, 0)));
            projectRect->setBrush(QBrush(QColor::fromHsv(qrand() % 256, 255, 190), Qt::SolidPattern));
            projectRect->setToolTip(project + " - " + QString::number(rank) + ' ' + i18n("commits"));
            projectRect->setAcceptHoverEvents(true);
            projectRect->installSceneEventFilter(kdeObservatory);

            QGraphicsPixmapItem *icon = new QGraphicsPixmapItem(KIcon(m_projects[project].icon).pixmap(22, 22), container);
            icon->setPos((qreal) widthFactor*rank+2, (qreal) yItem+((step-4)/2)-11);

            QGraphicsTextItem *commitsNumber = new QGraphicsTextItem(QString::number(rank), projectRect);
            commitsNumber->setDefaultTextColor(QColor(255, 255, 255));
            commitsNumber->setFont(KGlobalSettings::smallestReadableFont());
            commitsNumber->setPos((qreal) ((projectRect->rect().width())/2)-(commitsNumber->boundingRect().width()/2),
                                (qreal) ((projectRect->rect().height())/2)-(commitsNumber->boundingRect().height()/2));
            j++;
        }
    }
}
