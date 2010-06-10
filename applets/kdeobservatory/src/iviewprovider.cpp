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

#include "iviewprovider.h"

#include <QGraphicsLinearLayout>
#include <QGraphicsRectItem>

#include <Plasma/Frame>

IViewProvider::IViewProvider(KdeObservatory *kdeObservatory, QGraphicsWidget *parent, Qt::WindowFlags wFlags)
: m_kdeObservatory(kdeObservatory),
  m_parent(parent),
  m_wFlags(wFlags)
{
}

IViewProvider::~IViewProvider()
{
}

void IViewProvider::createView(const QString &title, const QString &id)
{
    QGraphicsWidget *view = new QGraphicsWidget(m_parent);
    view->setHandlesChildEvents(true);
    view->hide();

    Plasma::Frame *header = new Plasma::Frame(view);
    header->setText(title);
    header->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    QGraphicsWidget *container = new QGraphicsWidget(view);
    container->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    QGraphicsLinearLayout *layout = new QGraphicsLinearLayout(Qt::Vertical, view);
    layout->addItem(header);
    layout->addItem(container);

    view->setLayout(layout);
    view->setGeometry(m_parent->geometry());

    m_views.insert(id, view);
    m_viewContainers.insert(id, container);
}

QGraphicsWidget *IViewProvider::containerForView(const QString &id)
{
    return m_viewContainers[id];
}

void IViewProvider::deleteViews()
{
    foreach(QGraphicsWidget *view, m_views)
        view->deleteLater();
    m_views.clear();
}

const QList<QGraphicsWidget *> IViewProvider::views() const
{
    return m_views.values();
}
