/***************************************************************************
 *   Copyright (C) 2008-2012 Matthias Fuchs <mat69@gmx.net>                *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

#ifndef COMIC_WIDGETS_H
#define COMIC_WIDGETS_H

#include <Plasma/Label>
#include <Plasma/TabBar>

/**
 * The following classes are helper classes adpated to the desired
 * handling of the comic applet.
 */


//Helper class, sets the sizeHint to 0 if the TabBar is hidden
class ComicTabBar : public Plasma::TabBar
{
    public:
        ComicTabBar(QGraphicsWidget *parent = 0) : TabBar(parent) {}
        ~ComicTabBar() {}

        void removeAllTabs()
        {
            while (count()) {
                removeTab(0);
            }
        }

        bool hasHighlightedTabs() const
        {
            for (int i = 0; i < count(); ++i) {
                if (isTabHighlighted(i)) {
                    return true;
                }
            }

            return false;
        }

        int nextHighlightedTab(int index) const
        {
            int firstHighlighted = -1;
            for (int i = 0; i < count(); ++i) {
                if (isTabHighlighted(i)) {
                    if (i > index) {
                        return i;
                    } else if (firstHighlighted == -1) {
                        firstHighlighted = i;
                    }
                }
            }

            return (firstHighlighted != -1 ? firstHighlighted : index);
        }

    protected:
        QSizeF sizeHint(Qt::SizeHint which, const QSizeF &constraint = QSizeF()) const
        {
            if (!isVisible()) {
                return QSizeF(0, 0);
            }
            return QGraphicsWidget::sizeHint(which, constraint);
        }

        void hideEvent(QHideEvent *event)
        {
            updateGeometry();
            QGraphicsWidget::hideEvent(event);
        }

        void showEvent(QShowEvent*event)
        {
            updateGeometry();
            QGraphicsWidget::showEvent(event);
        }
};

//Helper class, sets the sizeHint to 0 if the Label is hidden
class ComicLabel : public Plasma::Label
{
    public:
        ComicLabel(QGraphicsWidget *parent = 0) : Plasma::Label(parent) {}
        ~ComicLabel() {}

    protected:
        QSizeF sizeHint(Qt::SizeHint which, const QSizeF &constraint = QSizeF()) const
        {
            if (!isVisible()) {
                return QSizeF(0, 0);
            }
            return QGraphicsProxyWidget::sizeHint(which, constraint);
        }

        void hideEvent(QHideEvent *event)
        {
            updateGeometry();
            QGraphicsProxyWidget::hideEvent(event);
        }

        void showEvent(QShowEvent *event)
        {
            updateGeometry();
            QGraphicsProxyWidget::showEvent(event);
        }
};

#endif
