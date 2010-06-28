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

#include "topactiveprojectsview.h"

#include <QPen>
#include <QFontMetrics>
#include <QLinearGradient>

#include <KIcon>
#include <KColorUtils>
#include <KGlobalSettings>

TopActiveProjectsView::TopActiveProjectsView(KdeObservatory *kdeObservatory, const QHash<QString, bool> &topActiveProjectsViewProjects, const QMap<QString, KdeObservatory::Project> &projects, QGraphicsWidget *parent, Qt::WindowFlags wFlags)
: IViewProvider(kdeObservatory, parent, wFlags),
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
    createView(i18n("Top Active Projects"), "Top Active Projects");
}

void TopActiveProjectsView::updateViews(const Plasma::DataEngine::Data &data)
{
    QMultiMap<int, QString> topActiveProjects = data["topActiveProjects"].value< QMultiMap<int, QString> >();

    QGraphicsWidget *container = containerForView("Top Active Projects");
    if (!container)
        return;

    int maxRank = 0;
    qreal width = container->geometry().width();
    qreal step = 22;

    QMapIterator<int, QString> i(topActiveProjects);
    i.toBack();
    int j = 0;
    while (i.hasPrevious())
    {
        i.previous();
        QString project = i.value();
        if (m_topActiveProjectsViewProjects[project])
        {
            int rank = i.key();
            if (j == 0)
                maxRank = rank;

            qreal widthFactor = (width-24)/maxRank;
            qreal yItem = (j*step)+2;

            if (yItem + step-4 > container->geometry().height())
                break;

            QGraphicsRectItem *projectRect = new QGraphicsRectItem(0, 0, (qreal) widthFactor*rank, (qreal) step-4, container);
            projectRect->setPos(0, yItem);
            projectRect->setPen(QPen(QColor(0, 0, 0)));
            QColor color = QColor::fromHsv(qrand() % 256, 255, 190);
            QLinearGradient grad (0.0, 0.0, (qreal) widthFactor*rank, 0.0);
            grad.setColorAt(0, KColorUtils::darken(color, 0.8));
            grad.setColorAt(1, color);
            projectRect->setBrush(QBrush(grad));
            projectRect->setToolTip(i18np("%2 - %1 commit", "%2 - %1 commits", rank, project));
            projectRect->setAcceptHoverEvents(true);
            projectRect->installSceneEventFilter(m_kdeObservatory);

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
