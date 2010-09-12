/*
 *   Copyright (C) 2009 by Ana Cecília Martins <anaceciliamb@gmail.com>
 *   Copyright (C) 2009 by Ivan Cukic <ivan.cukic+kde@gmail.com>
 *   Copyright (C) 2010 by Chani Armitage <chani@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library/Lesser General Public License
 *   version 2, or (at your option) any later version, as published by the
 *   Free Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library/Lesser General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

/**
 * Taken from kdebase/workspace/libs/plasmagenericshell
 */

#ifndef ABSTRACTICONLIST_H
#define ABSTRACTICONLIST_H

#include <QTimer>
#include <QGraphicsWidget>
#include <QGraphicsLinearLayout>
#include <QWeakPointer>

#include <Plasma/Plasma>
#include <Plasma/Svg>
#include <plasma/widgets/toolbutton.h>

#include "abstracticon.h"

namespace Plasma
{
    class Animation;
    class ItemBackground;
    class ToolButton;

class AbstractIconList : public QGraphicsWidget
{

    Q_OBJECT

public:
    AbstractIconList(Plasma::Location location = Plasma::BottomEdge, QGraphicsItem *parent = 0);
    ~AbstractIconList();

    void setLocation(Plasma::Location location);
    Plasma::Location location();

    // default size of the icons
    void setIconSize(int size);
    int iconSize() const;

protected:
    /**
     * add a new icon (or rather, do a bunchofstuff that needs doing when it's added)
     * this could do with some cleanup still.
     */
    void addIcon(AbstractIcon *applet);

    void showIcon(AbstractIcon *icon);

    void hideIcon(AbstractIcon *icon);

    void hideAllIcons();

    /**
     * subclasses must implement this:
     * it should show/hide icons as appropriate for the current filter.
     */
    virtual void updateVisibleIcons() = 0;

    /**
     * subclasses must implement this:
     * respond to changes in the search string (eg. tell your model)
     */
    virtual void setSearch(const QString &searchString) = 0;

    /**
     * scroll to a specific item
     */
    virtual void scrollTo(int index);

private:
    void init();

    //see how many icons is visible at once, approximately
    int maximumAproxVisibleIconsOnList();

    //removes all the icons from the widget
    void eraseList();

    //returns the what's the visible rect of the list widget
    QRectF visibleListRect();
    //returns window's start position
    qreal visibleStartPosition();
    //returns window's end position
    qreal visibleEndPosition();
    //returns list size
    qreal listSize();
    //returns windows size relative to list
    qreal windowSize();
    //returns item position
    qreal itemPosition(int i);

    void wheelEvent(QGraphicsSceneWheelEvent *event);

public Q_SLOTS:
    void searchTermChanged(const QString &text);
    void updateList();

protected:
    bool eventFilter(QObject *obj, QEvent *event);

    //Hash containing all widgets that represents the applets
    //FIXME the subclasses use this more than me so maybe they should store it?
    QHash<QString, AbstractIcon *> m_allAppletsHash;

protected Q_SLOTS:
    //checks if arrows should be enabled or not
    void manageArrows();

private Q_SLOTS:
    void scrollDownRight();
    void scrollUpLeft();
    void scrollStepFinished();

    //moves list to position 0,0
    void resetScroll();

    void itemSelected(Plasma::AbstractIcon *icon);
    void iconHoverEnter(Plasma::AbstractIcon *icon);

    void setSearch();

private:
    //list containing the applet icons of the filter proxy model
    QList<QWeakPointer<AbstractIcon > > m_currentAppearingAppletsOnList;

    QGraphicsLinearLayout *m_appletListLinearLayout;
    QGraphicsWidget *m_appletListWidget;
    QGraphicsWidget *m_appletListWindowWidget;
    QGraphicsLinearLayout *m_arrowsLayout;

    Plasma::ToolButton *m_downRightArrow;
    Plasma::ToolButton *m_upLeftArrow;
    Plasma::Svg *m_arrowsSvg;

    Qt::Orientation m_orientation;
    Plasma::Location m_location;

    Plasma::ItemBackground *m_hoverIndicator;

    //index of current first item
    //nothing to do with mvc indices.
    int m_firstItemIndex;

    AbstractIcon *m_selectedItem;

    QTimer *m_searchDelayTimer;
    QString m_searchString;

    int m_scrollStep;
    int m_iconSize;
    bool m_scrollingDueToWheel;

    Plasma::Animation *m_slide;
};
} // namespace Plasma

#endif //ICONLIST_H
