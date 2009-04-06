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

#ifndef TIMER_HEADER
#define TIMER_HEADER

#include <QTimer>

#include <Plasma/Applet>
#include <Plasma/Svg>

#include "ui_predefinedTimersConfig.h"
#include "ui_timerConfig.h"

class QGraphicsSceneMouseEvent;
class QMenu;
class CustomTimeEditor;
class QActionGroup;

class Timer : public Plasma::Applet
{
    Q_OBJECT
    public:
        Timer(QObject *parent, const QVariantList &args);
        ~Timer();

        void paintInterface(QPainter *painter,
                const QStyleOptionGraphicsItem *option,
                const QRect& contentsRect);
        void init();
        void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);
        void mousePressEvent(QGraphicsSceneMouseEvent * event);
        void wheelEvent(QGraphicsSceneWheelEvent * event);
        QList<QAction*> contextualActions();

    protected:
        void createConfigurationInterface(KConfigDialog *parent);
        void createMenuAction();
    private slots:
        void updateTimer();
        void slotCountDone();
        void startTimer();
        void stopTimer();
        void resetTimer();
        void startTimerFromAction();

    private:
        void saveTimer();

        int m_seconds;
        bool m_running;

        QTimer timer;
        Plasma::Svg *m_svg;
        QAction *m_startAction;
        QAction *m_stopAction;
        QAction *m_resetAction;
        Ui::predefinedTimersConfig predefinedTimersUi;
        Ui::timerConfig ui;

        QStringList m_predefinedTimers;
        bool m_showTitle;
        QString m_title;        
        bool m_showMessage;
        QString m_message;
        bool m_runCommand;
        QString m_command;
        QList<QAction *>actions;
        QActionGroup *lstActionTimer;
        QString m_separatorBasename;
    protected slots:
        void configAccepted();
};

K_EXPORT_PLASMA_APPLET(timer, Timer)
#endif
