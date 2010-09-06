/***************************************************************************
 *   Copyright 2008 by Davide Bettio <davide.bettio@kdemail.net>           *
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

#ifndef LUNA_H
#define LUNA_H

#include <Plasma/Applet>
#include <Plasma/DataEngine>
#include <Plasma/Svg>

#include "ui_lunaConfig.h"

class Luna : public Plasma::Applet
{
    Q_OBJECT
    public:
        Luna(QObject *parent, const QVariantList &args);
        ~Luna();

        void init();
        void paintInterface(QPainter *painter, const QStyleOptionGraphicsItem *option, const QRect& contentsRect);

    public slots:
        void dataUpdated(const QString &name, const Plasma::DataEngine::Data &data);
        void configChanged();
        
    protected slots:
        void configAccepted();

    protected:
        void createConfigurationInterface(KConfigDialog *parent);

    private:
        void connectToEngine();
        void calcStatus(time_t time);

        int counter;
        bool northHemisphere;

        Plasma::Svg *m_theme;
        Ui::lunaConfig ui;
};

K_EXPORT_PLASMA_APPLET(luna, Luna)

#endif
