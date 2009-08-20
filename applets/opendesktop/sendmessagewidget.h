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

#include <Plasma/DataEngine>
#include <Plasma/Frame>
#include <Plasma/Label>
#include <Plasma/PushButton>
#include <Plasma/TextEdit>


class SendMessageWidget : public Plasma::Frame
{
    Q_OBJECT

    public:
        SendMessageWidget(Plasma::DataEngine* engine, const QString& id, QGraphicsWidget* parent = 0);

    Q_SIGNALS:
        void done();

    private Q_SLOTS:
        void dataUpdated(const QString& source, const Plasma::DataEngine::Data& data);
        void send();

    private:
        Plasma::TextEdit* m_body;
        Plasma::DataEngine* m_engine;
        const QString m_id;
        const QString m_query;
        Plasma::Label* m_label;
        Plasma::TextEdit* m_subject;
        Plasma::PushButton* m_submit;
};


#endif
