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

#ifndef LIFE_H
#define LIFE_H

#include <Plasma/Applet>
#include <Plasma/Svg>
#include <QTimer>

#include "ui_lifeConfig.h"

namespace Plasma
{
    class DataEngine;
}
namespace Ui
{
    class lifeConfig;
}

class Life : public Plasma::Applet
{
    Q_OBJECT
    public:
        Life(QObject *parent, const QVariantList &args);
        ~Life();

        void init();
        void paintInterface(QPainter *painter, const QStyleOptionGraphicsItem *option, const QRect& contentsRect);

    public slots:
        void createConfigurationInterface(KConfigDialog *parent);

    protected slots:
        void configAccepted();

    private slots:
        void updateGame();

    private:
        void startUpdateTimer();

        int neighbors(int i);
        void step();
        void initGame();
        void resetGame();

        char *cells;
        char *nextGenerationCells;

        int cellsArrayHeight;
        int cellsArrayWidth;

        int stepInterval;
        int maxGensNumber;

        int gensCounter;

        Ui::lifeConfig ui;
        QTimer timer;
};

K_EXPORT_PLASMA_APPLET(life, Life)

#endif
