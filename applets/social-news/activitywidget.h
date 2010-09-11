/***************************************************************************
 *   Copyright 2009 by Marco Martin <notmart@gmail.com>                    *
 *   Copyright 2010 Frederik Gladhorn <gladhorn@kde.org>                   *
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

#ifndef ACTIVITYWIDGET_H
#define ACTIVITYWIDGET_H

#include <Plasma/Frame>
#include <Plasma/DataEngine>

class QGraphicsLinearLayout;

class ContactImage;

namespace Plasma
{
    class IconWidget;
    class Label;
}


class ActivityWidget : public Plasma::Frame
{
    Q_OBJECT

public:
    explicit ActivityWidget(Plasma::DataEngine* engine, QGraphicsWidget *parent = 0);
    virtual ~ActivityWidget();
    
    QDateTime timestamp() const;
    QString message() const;

public Q_SLOTS:
    void setActivityData(Plasma::DataEngine::Data data);
    Plasma::DataEngine::Data activityData() const;

protected Q_SLOTS:
    void hoverEnterEvent(QGraphicsSceneHoverEvent* event);
    void hoverLeaveEvent(QGraphicsSceneHoverEvent* event);

private Q_SLOTS:
    void followLink();

private:
    void updateActions();
    Plasma::DataEngine::Data m_atticaData;
    QGraphicsLinearLayout *m_layout;
    Plasma::Label *m_messageLabel;
    ContactImage* m_image;
    Plasma::IconWidget* m_link;
    QDateTime m_timestamp;
    bool m_isHovered;
};

#endif

