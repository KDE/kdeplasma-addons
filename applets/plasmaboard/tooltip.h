/****************************************************************************
 *   Copyright (C) 2009 by Björn Ruberg <bjoern@ruberg-wegener.de>    *
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

#ifndef TOOLTIP_H
#define TOOLTIP_H

#include <QWidget>
#include <plasma/framesvg.h>

class QLabel;
class QHBoxLayout;

class Tooltip : public QWidget
{
    Q_OBJECT
public:
    Tooltip(QString text);
    ~Tooltip();
    void setText(QString text);
    void resize(QSize size);
protected:
    void paintEvent ( QPaintEvent * event );
    bool event(QEvent *event);
private:
    Plasma::FrameSvg* frame;
    QLabel* label;
    QHBoxLayout* m_layout;

  public Q_SLOTS:
    void setColors();
};

#endif // TOOLTIP_H
