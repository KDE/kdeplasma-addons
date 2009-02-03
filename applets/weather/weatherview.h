/*
 *   Copyright 2008 Marco Martin <notmart@gmail.com>
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


#ifndef PLASMA_WEATHERVIEW_H
#define PLASMA_WEATHERVIEW_H

#include <plasma/widgets/treeview.h>

class WeatherDelegate;

namespace Plasma
{

class WeatherView : public TreeView
{
    Q_OBJECT

public:
    explicit WeatherView(QGraphicsWidget *parent = 0);
    ~WeatherView();

    void setHasHeader(bool hasHeader);
    bool hasHeader() const;

    void setOrientation(Qt::Orientation orientation);
    Qt::Orientation orientation() const;

protected:
    void resizeEvent(QGraphicsSceneResizeEvent *event);

private:
    WeatherDelegate *m_delegate;

};

}

#endif // multiple inclusion guard
