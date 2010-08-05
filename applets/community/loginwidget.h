/*
    This file is part of KDE.

    Copyright (c) 2009 Frederik Gladhorn <gladhorn@kde.org>

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

#ifndef LOGINWIDGET_H
#define LOGINWIDGET_H

#include <QtCore/QTimer>


#include <Plasma/DataEngine>
#include <Plasma/Frame>
#include <Plasma/Label>
#include <Plasma/LineEdit>
#include <Plasma/PushButton>
#include <Plasma/TextEdit>

#include "personwatch.h"


class KJob;

class LoginWidget : public QGraphicsWidget
{
    Q_OBJECT

    public:
        LoginWidget(Plasma::DataEngine* engine, QGraphicsWidget* parent = 0);
    
    public Q_SLOTS:
        void setProvider(const QString& provider);
    
    Q_SIGNALS:
        void loginFinished();

    private Q_SLOTS:
        void login();
        void registerNewAccount();
        void loginJobFinished(KJob* job);

    private:
        
        Plasma::Label* m_serverLabel;
        Plasma::Label* m_userLabel;
        Plasma::Label* m_passwordLabel;
        Plasma::LineEdit* m_userEdit;
        Plasma::LineEdit* m_passwordEdit;
        
        Plasma::DataEngine* m_engine;
        QString m_id;
        QString m_provider;
};


#endif
