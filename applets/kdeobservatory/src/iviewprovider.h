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

#ifndef IVIEWPROVIDER_HEADER
#define IVIEWPROVIDER_HEADER

#include <QGraphicsWidget>

#include <Plasma/DataEngine>

class QGraphicsWidget;

namespace Plasma
{
    class Frame;
}

class KdeObservatory;

class IViewProvider : public QObject
{
    Q_OBJECT
public:
    explicit IViewProvider(KdeObservatory *kdeObservatory, QGraphicsWidget *parent = 0, Qt::WindowFlags wFlags = 0);
    virtual ~IViewProvider();

    void createView(const QString &title, const QString &id);
    QGraphicsWidget *containerForView(const QString &id);
    void deleteViews();
    const QList<QGraphicsWidget *> views() const;

    virtual void createViews() = 0;
    virtual void updateViews(const Plasma::DataEngine::Data &data) = 0;

protected:
    KdeObservatory *m_kdeObservatory;
    QGraphicsWidget *m_parent;
    Qt::WindowFlags m_wFlags;
    QMap<QString, QGraphicsWidget *> m_views;
    QHash<QString, QGraphicsWidget *> m_viewContainers;
};

#endif
