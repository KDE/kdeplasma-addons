/***************************************************************************
 *   Copyright 2008-2009 by Sebastian Kügler <sebas@kde.org>               *
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

#ifndef CONTACTWIDGET_H
#define CONTACTWIDGET_H

//Qt
#include <QGraphicsGridLayout>
#include <QGraphicsLinearLayout>
#include <QGraphicsWidget>

#include <Plasma/PopupApplet>
#include <Plasma/Frame>
#include <Plasma/DataEngine>

namespace Plasma
{
    class IconWidget;
    class Dialog;
    class Label;
}

class ContactImage;

class ContactWidget : public Plasma::Frame
{
    Q_OBJECT

    public:
        explicit ContactWidget(Plasma::DataEngine* engine, QGraphicsWidget *parent = 0);
        virtual ~ContactWidget();

        QString id() const;
        QString name() const;
        QString provider() const;
        QString user() const;

        void setName(const QString &name);
        void setInfo(const QString &name);
        void setIsFriend(bool isFriend);
        void setId(const QString& id);
        void setProvider(const QString& provider);

        Plasma::PopupApplet* m_applet;

    Q_SIGNALS:
        void addFriend();
        void sendMessage();
        void showDetails();

    public Q_SLOTS:
        void updateColors();

    protected:
        void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
        void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);
        void mousePressEvent(QGraphicsSceneMouseEvent* event);
        void mouseReleaseEvent(QGraphicsSceneMouseEvent* event);
        void mouseMoveEvent(QGraphicsSceneMouseEvent *event);

    protected Q_SLOTS:
        void slotShowDetails();
        void dataUpdated(const QString& source, const Plasma::DataEngine::Data& data);

    private :
        void buildDialog();
        void updateActions();

        Plasma::DataEngine::Data m_ocsData;
        QString m_stylesheet;
        bool m_isHovered;
        bool m_isFriend;
        // The applet attached to this item
        QGraphicsGridLayout* m_layout;
        QGraphicsLinearLayout* m_actions;
        ContactImage* m_image;
        Plasma::Label* m_nameLabel;
        Plasma::Label* m_infoLabel;
        Plasma::IconWidget* m_sendMessage;
        Plasma::IconWidget* m_addFriend;
        Plasma::IconWidget* m_showDetails;
        Plasma::DataEngine* m_engine;
        QString m_id;
        QString m_provider;
};

#endif

