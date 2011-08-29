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
#include <Plasma/FrameSvg>
#include <Plasma/WindowEffects>

class QWidget;
class QLabel;
class QHBoxLayout;

class Tooltip : public QWidget
{
    Q_OBJECT
public:
    Tooltip(const QString &text = QString());
    ~Tooltip();
    void setText(QString text);

protected:
    void paintEvent(QPaintEvent * event);
    void resizeEvent(QResizeEvent *event);
    void showEvent(QShowEvent * event);

public Q_SLOTS:
    void setColors();

protected Q_SLOTS:
    void updateMask();

private:
    Plasma::FrameSvg* frame;
    QLabel* label;
    QHBoxLayout* m_layout;

};

#endif // TOOLTIP_H
