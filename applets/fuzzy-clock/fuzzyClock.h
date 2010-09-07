/***************************************************************************
 *   Copyright (C) 2007 by Sven Burmeister <sven.burmeister@gmx.net>       *
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

#ifndef CLOCK_H
#define CLOCK_H

#include <Plasma/DataEngine>

#include <plasmaclock/clockapplet.h>

#include "ui_fuzzyClockConfig.h"

class QTime;
class QDate;

class KLocalizedString;

class Clock : public ClockApplet
{
    Q_OBJECT
    public:
        Clock(QObject *parent, const QVariantList &args);
        ~Clock();

        void init();
        void paintInterface(QPainter *painter, const QStyleOptionGraphicsItem *option, const QRect &contentsRect);
        void setPath(const QString&);
//         QSizeF contentSizeHint() const;

Qt::Orientations expandingDirections() const;

    public slots:
        void dataUpdated(const QString &name, const Plasma::DataEngine::Data &data);

    protected slots:
//         void acceptedTimeStringState(bool);
           void updateColors();

    protected:
        void constraintsEvent(Plasma::Constraints constraints);
        void createClockConfigurationInterface(KConfigDialog *parent);
        void clockConfigAccepted();
        void clockConfigChanged();
        void changeEngineTimezone(const QString &oldTimezone, const QString &newTimezone);

    private:
        void initFuzzyTimeStrings();

        void calculateTimeString();
        void calculateDateString();
        void calculateSize();

        // temporary, sort out a correct way for applets to be notified
        // when their content size changes and then rather than tracking
        // the content size, re-implement the appropriate method to
        // update the graphic sizes and so on
        QSizeF m_contentSize;
        QSizeF m_oldContentSize;
        QSizeF m_minimumContentSize;

        bool m_configUpdated;

        QString m_timeString;
        QString m_dateString;
        QString m_timezoneString;
        QString m_subtitleString;

        QSizeF m_timeStringSize;
        QSizeF m_dateStringSize;
        QSizeF m_timezoneStringSize;
        QSizeF m_subtitleStringSize;

        int m_adjustToHeight;
        bool m_useCustomFontColor;
        QColor m_fontColor;
        bool m_fontTimeBold;
        bool m_fontTimeItalic;

        QFont m_fontTime;
        QFont m_fontDate;

//         QFontMetrics m_fmTime;
//         QFontMetrics m_fmDate;

        int m_fuzzyness;
        bool m_showTimezone;
        bool m_showDate;
        bool m_showYear;
        bool m_showDay;
        QTime m_time;
        QDate m_date;
        KLocale *m_locale;
        QVBoxLayout *m_layout;

        QTime m_lastTimeSeen;
        QString m_lastTimeStringSeen;
        QString m_lastDateStringSeen;

        /// Designer Config file
        Ui::fuzzyClockConfig ui;

        QStringList m_hourNames;
        QList<KLocalizedString> m_normalFuzzy;
        QStringList m_dayTime;
        QStringList m_weekTime;

        int m_margin;
        int m_verticalSpacing;
};

K_EXPORT_PLASMA_APPLET(fuzzy_clock, Clock)

#endif
