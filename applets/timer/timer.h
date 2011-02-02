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

#include <QtCore/QList>
#include <QtCore/QTimer>

#include <Plasma/Applet>

#include "ui_predefinedTimersConfig.h"
#include "ui_timerConfig.h"
#include "timerdigit.h"

class QGraphicsSceneMouseEvent;
class QActionGroup;
class QAbstractAnimation;

namespace Plasma
{
    class Label;
    class Svg;
    class SvgWidget;
}

class Timer : public Plasma::Applet
{
    Q_OBJECT
    Q_PROPERTY(qreal digitOpacity READ digitOpacity WRITE setDigitOpacity)

    public:
        Timer(QObject *parent, const QVariantList &args);
        ~Timer();

        void init();
        void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);
        void mousePressEvent(QGraphicsSceneMouseEvent * event);
        void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
        QList<QAction*> contextualActions();

        qreal digitOpacity() const;
        void setDigitOpacity(qreal opacity);

    protected:
        void createConfigurationInterface(KConfigDialog *parent);
        void createMenuAction();
        void constraintsEvent(Plasma::Constraints constraints);

    public slots:
        void configChanged();

    private slots:
        void updateTimer();
        void slotCountDone();
        void startTimer();
        void stopTimer();
        void resetTimer();
        void startTimerFromAction();
        void digitChanged(int value);
        void toggleTimerVisible();
        void reverseBlinkAnim();

    private:
        void setBlinking(bool blinking);
        void saveTimer();
        void setSeconds(int seconds);

        int m_seconds;
        int m_startingSeconds;
        bool m_running;

        QTimer m_timer;
        QAbstractAnimation *m_blinkAnim;
        Plasma::Svg *m_svg;
        TimerDigit *m_hoursDigit[2];
        TimerDigit *m_minutesDigit[2];
        TimerDigit *m_secondsDigit[2];
        Plasma::SvgWidget *m_separator[2];
        Plasma::Label *m_title;

        QAction *m_startAction;
        QAction *m_stopAction;
        QAction *m_resetAction;
        Ui::predefinedTimersConfig predefinedTimersUi;
        Ui::timerConfig ui;

        QStringList m_predefinedTimers;    
        bool m_showMessage;
        QString m_message;
        bool m_runCommand;
        QString m_command;
        QList<QAction *>actions;
        QActionGroup *lstActionTimer;
        QString m_separatorBasename;
        QDateTime m_startedAt;
        bool m_showTitle;
        QString m_timerTitle;
        bool m_hideSeconds;

    protected slots:
        void configAccepted();
};

K_EXPORT_PLASMA_APPLET(timer, Timer)
#endif
