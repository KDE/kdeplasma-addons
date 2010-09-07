/***************************************************************************
 *   Copyright 2007 by André Duffeck <duffeck@kde.org>                     *
 *   Copyright 2007 by Alexis Ménard <darktears31@gmail.com>               *
 *   Copyright 2007 by Marco Martin <notmart@gmail.com>                    *
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

#ifndef SHOWDASHBOARD_H
#define SHOWDASHBOARD_H


#include <Plasma/Applet>
#include <QTimer>
#include <QGraphicsSceneDragDropEvent>


class ShowDashboard : public Plasma::Applet
{
    Q_OBJECT
    public:
        ShowDashboard(QObject *parent, const QVariantList &args);
        ~ShowDashboard() {};
        void init();

    protected slots:
        void toggleShowDashboard(bool);
        void toggleShowDashboard();

    protected:
        void dragEnterEvent(QGraphicsSceneDragDropEvent *event);
        void dragLeaveEvent(QGraphicsSceneDragDropEvent *event);
        QSizeF sizeHint(Qt::SizeHint which, const QSizeF & constraint = QSizeF()) const;
        
    private slots:
        void iconSizeChanged(int group);

    private:
        QTimer m_timer;
};

K_EXPORT_PLASMA_APPLET(showdashboard, ShowDashboard)

#endif
