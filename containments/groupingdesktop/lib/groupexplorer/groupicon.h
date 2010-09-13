/*
 *   Copyright 2010 by Giulio Camuffo <giuliocamuffo@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef GROUPICON_H
#define GROUPICON_H

#include <KIcon>

#include "abstracticon.h"

class AbstractGroup;

class GroupIcon : public Plasma::AbstractIcon
{
    Q_OBJECT
    public:
        explicit GroupIcon(const QString &name);
        virtual ~GroupIcon();

        QPixmap pixmap(const QSize &size);
        QMimeData* mimeData();

    private:
        QString m_name;
        KIcon m_icon;
};

#endif //GROUPICON_H
