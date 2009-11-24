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

#ifndef SENDMESSAGEWIDGET_H
#define SENDMESSAGEWIDGET_H

#include <QtCore/QTimer>

#include <Plasma/DataEngine>
#include <Plasma/Frame>
#include <Plasma/Label>
#include <Plasma/LineEdit>
#include <Plasma/PushButton>
#include <Plasma/TextEdit>

#include "personwatch.h"


class ContactImage;

namespace Plasma {
    class IconWidget;
}

class SendMessageWidget : public Plasma::Frame
{
    Q_OBJECT

    public:
        SendMessageWidget(Plasma::DataEngine* engine, QGraphicsWidget* parent = 0);

    public Q_SLOTS:
        void setId(const QString& id);
        void setProvider(const QString& provider);

    Q_SIGNALS:
        void endWork();
        void done();
        void startWork();

    private Q_SLOTS:
        void personUpdated();
        void send();
        void switchToBody();
        void switchToSubject();
        void toChanged(const QString& to);
        void updateTo();
        void updateSendAction();

    private:
        Plasma::LineEdit* m_toEdit;
        Plasma::TextEdit* m_body;
        Plasma::DataEngine* m_engine;
        Plasma::Label* m_toLabel;
        Plasma::LineEdit* m_subject;
        Plasma::IconWidget* m_submit;
        QString m_id;
        QString m_provider;
        QTimer m_updateTimer;
        PersonWatch m_personWatch;
        ContactImage* m_image;
};


#endif
