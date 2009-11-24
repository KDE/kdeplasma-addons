/*
    This file is part of KDE.

    Copyright (c) 2009 Eckhart Wörner <ewoerner@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,
    USA.
*/

#ifndef FRIENDMANAGEMENTWIDGET_H
#define FRIENDMANAGEMENTWIDGET_H

#include <Plasma/Frame>

#include "personwatch.h"


class ContactImage;
class QGraphicsGridLayout;
class QGraphicsLinearLayout;

namespace Plasma {
    class IconWidget;
    class Label;
}

class FriendManagementWidget : public Plasma::Frame
{
    Q_OBJECT

public:
    explicit FriendManagementWidget(Plasma::DataEngine* engine, QGraphicsWidget* parent = 0);

public Q_SLOTS:
    void setId(const QString& id);
    void setProvider(const QString& provider);

Q_SIGNALS:
    void showDetails();

protected Q_SLOTS:
    void accept();
    void decline();
    void updated();
    void hoverEnterEvent(QGraphicsSceneHoverEvent* event);
    void hoverLeaveEvent(QGraphicsSceneHoverEvent* event);

private:
    void buildDialog();
    void updateActions();

    bool m_isHovered;

    QGraphicsGridLayout* layout;
    QGraphicsLinearLayout* m_actions;
    ContactImage* m_avatar;
    Plasma::Label* m_infoLabel;
    Plasma::Label* m_statusLabel;
    Plasma::IconWidget* m_actionAccept;
    Plasma::IconWidget* m_actionDecline;
    QString m_id;
    QString m_provider;
    PersonWatch m_personWatch;
    Plasma::DataEngine* m_engine;
};


#endif
