/***************************************************************************
 *   Copyright 2007 by Riccardo Iaconelli <riccardo@kde.org>               *
 *   Copyright 2007 by Davide Bettio <davide.bettio@kdemail.net>           *
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

#ifndef BINARYCLOCK_H
#define BINARYCLOCK_H

#include <Plasma/Applet>

class QTime;
class QColor;

class Plasma::DataEngine;

class Ui::clockConfig;

class BinaryClock : public Plasma::Applet
{
    Q_OBJECT
    public:
        BinaryClock(QObject *parent, const QVariantList &args);
        ~BinaryClock();

        void init();
        void paintInterface(QPainter *painter, const QStyleOptionGraphicsItem *option, const QRect& contentsRect);
        void constraintsEvent(Plasma::Constraints constraints);

    public slots:
        void dataUpdated(const QString &name, const Plasma::DataEngine::Data &data);

    protected slots:
        void configAccepted();

    protected:
        void createConfigurationInterface(KConfigDialog *parent);

    private:
        void connectToEngine();

        int getWidthFromHeight(int h) const;
        int getHeightFromWidth(int w) const;

        bool m_showSeconds;
        bool m_showOffLeds;
        bool m_showGrid;

        QString m_timezone;

        QColor m_ledsColor;
        QColor m_offLedsColor;
        QColor m_gridColor;

        QTime m_lastTimeSeen;
        QTime m_time;

        Ui::clockConfig ui;
};

K_EXPORT_PLASMA_APPLET(binaryclock, BinaryClock)

#endif
