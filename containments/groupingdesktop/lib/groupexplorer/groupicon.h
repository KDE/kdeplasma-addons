/*
 *   Copyright (C) 2009 by Ana Cecília Martins <anaceciliamb@gmail.com>
 *   Copyright (C) 2010 by Chani Armitage <chani@kde.org>
 *   Copyright 2010 by Giulio Camuffo <giuliocamuffo@gmail.com>
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

#ifndef GROUPICON_H
#define GROUPICON_H

#include <QGraphicsWidget>

#include <KIcon>

namespace Plasma {
    class FrameSvg;
}

class GroupIcon : public QGraphicsWidget
{
    Q_OBJECT

    public:
        GroupIcon(const QString &name);
        ~GroupIcon();

        void setIconSize(int height);
        void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0);

        static const int DEFAULT_ICON_SIZE = 16;

    Q_SIGNALS:
        void hoverEnter(GroupIcon *icon);

    protected:
        //listen to events and emit signals
        void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
        void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
        void mousePressEvent(QGraphicsSceneMouseEvent *event);
        void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
        void resizeEvent(QGraphicsSceneResizeEvent *event);
        QSizeF sizeHint(Qt::SizeHint which, const QSizeF &constraint) const;

        bool isDraggable() const;

    private:
        Plasma::FrameSvg *m_background;
        QString m_name;
        QString m_id;
        KIcon m_icon;
        int m_iconHeight;
};

#endif //GROUPICON_H
