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
        ContactWidget(QGraphicsWidget *parent = 0);
        virtual ~ContactWidget();

        void setAtticaData(const Plasma::DataEngine::Data &data);

        QString name();
        QString user();

        void setName(const QString &name);
        void setInfo(const QString &name);
        void setIsFriend(bool isFriend);
        void setImage(const QImage &image);
        Plasma::DataEngine::Data data();

        qlonglong id;

        Plasma::PopupApplet* m_applet;

    Q_SIGNALS:
        void showDetails(Plasma::DataEngine::Data data);

    public Q_SLOTS:
        void updateColors();
        void sendMessage();
        void addFriend();
        void userProperties();

    protected Q_SLOTS:
        void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
        void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);

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
};

#endif

