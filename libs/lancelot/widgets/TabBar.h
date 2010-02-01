/*
 *   Copyright (C) 2010 Ivan Cukic <ivan.cukic(at)kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Lesser/Library General Public License version 2,
 *   or (at your option) any later version, as published by the Free
 *   Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU Lesser/Library General Public License for more details
 *
 *   You should have received a copy of the GNU Lesser/Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef LANCELOT_TAB_BAR_H
#define LANCELOT_TAB_BAR_H

#include <QtGui/QIcon>

#include <Plasma/Plasma>
#include <Plasma/ItemBackground>
#include <Plasma/FrameSvg>

#include <lancelot/lancelot_export.h>

namespace Lancelot
{

/**
 * @author Ivan Cukic
 */
class LANCELOT_EXPORT TabBar: public QGraphicsWidget {
    Q_OBJECT

    Q_PROPERTY(Qt::Orientation orientation READ orientation WRITE setOrientation)
    Q_PROPERTY(Qt::Orientation textDirection READ textDirection WRITE setTextDirection)
    Q_PROPERTY(QString currentTab READ currentTab WRITE setCurrentTab)

    // @puck L_WIDGET
    // @puck L_INCLUDE(lancelot/widgets/BasicWidget.h QIcon QSize QString)

public:
    TabBar(QGraphicsWidget * parent = 0);
    ~TabBar();

    Qt::Orientation orientation() const;
    void setOrientation(Qt::Orientation value);

    Qt::Orientation textDirection() const;
    void setTextDirection(Qt::Orientation value);

    QString currentTab() const;

    void addTab(const QString & id, const QIcon & icon, const QString & title);
    void removeTab(const QString & id);

    void setTabsGroupName(const QString & groupName);

    void paint(QPainter * painter,
        const QStyleOptionGraphicsItem * option,
        QWidget * widget = 0);

protected:
    L_Override void resizeEvent(QGraphicsSceneResizeEvent * event);

Q_SIGNALS:
    void currentTabChanged(const QString & current);

public Q_SLOTS:
    void setCurrentTab(const QString & current);

private:
    class Private;
    Private * const d;

};

} // namespace Lancelot

#endif /* LANCELOT_TAB_BAR_H */

