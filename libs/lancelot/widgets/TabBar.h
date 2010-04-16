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
 * Provides a group of buttons representing tabs. Can
 * be used for controlling the CardLayout
 *
 * @author Ivan Cukic
 */
class LANCELOT_EXPORT TabBar: public QGraphicsWidget {
    Q_OBJECT

    Q_PROPERTY(Qt::Orientation orientation READ orientation WRITE setOrientation)
    Q_PROPERTY(Qt::Orientation textDirection READ textDirection WRITE setTextDirection)
    Q_PROPERTY(QString currentTab READ currentTab WRITE setCurrentTab)
    Q_PROPERTY(QSize tabIconSize READ tabIconSize WRITE setTabIconSize)

    // @puck L_WIDGET
    // @puck L_INCLUDE(lancelot/widgets/TabBar.h QIcon QSize QString)

public:
    /**
     * Creates a new Lancelot::TabBar
     * @param parent parent item
     */
    TabBar(QGraphicsWidget * parent = 0);

    /**
     * Destroys this Lancelot::TabBar
     */
    ~TabBar();

    /**
     * @returns the tab bar orientation
     */
    Qt::Orientation orientation() const;

    /**
     * Sets the tab bar orientation
     * @param value new orientation
     */
    void setOrientation(Qt::Orientation value);

    /**
     * @returns the inner layout of buttons
     * @see setTextDirection
     */
    Qt::Orientation textDirection() const;

    /**
     * Sets the inner layout of tab buttons - that is
     * whether the icon is above or beside the text
     */
    void setTextDirection(Qt::Orientation value);

    /**
     * @returns the active tab
     */
    QString currentTab() const;

    /**
     * Adds a new tab
     * @param id id of the tab
     * @param icon icon for the tab
     * @param title tab title
     */
    void addTab(const QString & id, const QIcon & icon, const QString & title,
            const QString & mimeType = QString::null, const QString & mimeData = QString::null);

    /**
     * Removes the specified tab
     * @param id id of the tab to remove
     */
    void removeTab(const QString & id);

    /**
     * Sets the Lancelot::Group for the tab buttons
     * @param groupName name of the specific Lancelot::Group
     */
    void setTabsGroupName(const QString & groupName);

    /**
     * Sets the layout flip
     * @param flip new value
     */
    void setFlip(Plasma::Flip flip);

    /**
     * @returns the current layout flip
     */
    Plasma::Flip flip() const;

    /**
     * Sets the icon size for tab buttons
     * @param size new size
     */
    void setTabIconSize(const QSize & size);

    /**
     * @returns the current icon size of tab buttons
     */
    QSize tabIconSize() const;

protected:
    L_Override void resizeEvent(QGraphicsSceneResizeEvent * event);
    L_Override bool sceneEventFilter(QGraphicsItem * watched, QEvent * event);


Q_SIGNALS:
    /**
     * This signal is emitted when the currently selected
     * tab is changed
     */
    void currentTabChanged(const QString & current);

public Q_SLOTS:
    /**
     * Sets the current tab
     */
    void setCurrentTab(const QString & current);

private:
    class Private;
    Private * const d;

};

} // namespace Lancelot

#endif /* LANCELOT_TAB_BAR_H */

