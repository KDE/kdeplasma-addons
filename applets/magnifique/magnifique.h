/***************************************************************************
 *   Copyright 2008 by Marco Martin <notmart@gmail.com>                    *
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

#ifndef MAGNIFIQUE_HEADER
#define MAGNIFIQUE_HEADER



#include <Plasma/Applet>


class QGraphicsView;
class QSlider;
class QWidget;

class Magnifique : public Plasma::Applet
{
Q_OBJECT
public:

     Magnifique(QObject *parent, const QVariantList &args);
     ~Magnifique();


     void init();

protected:
     bool eventFilter(QObject *watched, QEvent *event);
     void wheelEvent(QGraphicsSceneWheelEvent *event);

public Q_SLOTS:
    void toggleView();
    void setZoom(int zoom);

private:
    void syncViewToScene();

    QPointF scenePosFromScreenPos(const QPoint &pos) const;

    QGraphicsView *m_view;
    QWidget *m_mainWindow;
    QSlider *m_slider;
};


K_EXPORT_PLASMA_APPLET(magnifique, Magnifique)
#endif
