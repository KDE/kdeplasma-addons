/***************************************************************************
 *   Copyright (c) 1996-2002 the kicker authors. (fuzzy logic)             *
 *   Copyright (C) 2005,2006,2007 by Siraj Razick <siraj@kdemail.net>      *
 *   Copyright (C) 2007 by Riccardo Iaconelli <ruphy@fsfe.org>             *
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

#include "fuzzyClock.h"

#include <math.h>

#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <QFontMetrics>
#include <KGlobalSettings>
#include <QtGui/QGraphicsView>
#include <QtGui/QGraphicsSceneMouseEvent>

#include <KDebug>
#include <KLocale>
#include <KSharedConfig>
#include <KDialog>

#include <KSystemTimeZones>



Clock::Clock(QObject *parent, const QVariantList &args)
    : Plasma::Applet(parent, args),
      m_fontColor(Qt::white),
      m_fontTimeBold(false),
      m_adjustToHeight(1),
      m_fontTime(KGlobalSettings::smallestReadableFont()),
      m_showDate(false),
      m_showYear(false),
      m_showDay(false),
      m_showTimezone(false),
      m_dialog(0),
      m_calendar(0),
      m_layout(0)
{
    setHasConfigurationInterface(true);
    setDrawStandardBackground(true);
}

void Clock::init()
{
    m_oldContentSize = QSizeF (0,0);
    m_contentSize = contentSize();

    kDebug() << "The first contentSize() we get, init() called: " << contentSize();

    m_localTimezone = KSystemTimeZones::local();
    m_locale = KGlobal::locale();

    KConfigGroup cg = config();

    m_showTimezone = cg.readEntry("showTimezone", false);
    m_showDate = cg.readEntry("showDate", true);
    m_showYear = cg.readEntry("showYear",false);
    m_showDay = cg.readEntry("showDay",true);

    m_useLocalTimezone = cg.readEntry("useLocalTimezone",true);
    m_timezone = KSystemTimeZones::zone ( cg.readEntry("timezone", m_localTimezone.name() )); 

    m_fuzzyness = cg.readEntry("fuzzyness", 1);

    m_fontTime = cg.readEntry("fontTime", KGlobalSettings::smallestReadableFont());
    m_fontColor = cg.readEntry("fontColor", m_fontColor);
    m_fontTimeBold = cg.readEntry("fontTimeBold", true);

    m_fontTime.setBold(m_fontTimeBold);

    m_adjustToHeight = cg.readEntry("adjustToHeight", 1);

    //By default we use the smallest readable font.
    m_fontDate = QFont ( KGlobalSettings::smallestReadableFont() );

    m_margin = 2;
    m_verticalSpacing = 2;

    if( m_timezone.name().isEmpty() ) {
        m_timezone = m_localTimezone;
    }

    Plasma::DataEngine* timeEngine = dataEngine("time");
    timeEngine->connectSource(m_timezone.name(), this, 6000, Plasma::AlignToMinute);
}

Qt::Orientations Clock::expandingDirections() const
{
    return Qt::Vertical;
}

// QSizeF Clock::contentSizeHint() const
// {
//     return contentSize();
// }

void Clock::constraintsUpdated(Plasma::Constraints constraints)
{
    kDebug() << "constraintsUpdated() called";

    m_fixedHeight = false;

    if (m_oldContentSize.toSize() != contentSize().toSize() || m_configUpdated == true ) { //The size changed or config was updated
        kDebug() << "The contentSize() changed! old: " << m_oldContentSize << "new: " << contentSize();

        if ( m_configUpdated ) {
            calculateDateString();
            calculateTimeString();
        }

        //Contraints have changed, e.g. size.
        if (constraints & Plasma::SizeConstraint) {

            kDebug() << "Constraints changed: " << constraints;

            if (formFactor() == Plasma::Planar || formFactor() == Plasma::MediaCenter) {
                kDebug() << "######## Other FormFactor";

                calculateSize();
            } else {
                kDebug() << "######## Small FormFactor";

                //In case of the panel we have to accept the height, even if we do not need it! Otherwise constraintsUpdated gets called again and again with the smae height over and over again.
                m_fixedHeight = true;
                calculateSize();
            }

        kDebug() << "The new size has been calculated. needed: " << m_contentSize << "\nactual contentSize() is: " << contentSize() << "\nminimumContentSize() needed: " << minimumContentSize();

        m_oldContentSize = contentSize();
        m_configUpdated = false;

        update();
        }
    }
}

void Clock::dataUpdated(const QString& source, const Plasma::DataEngine::Data &data)
{
    Q_UNUSED(source);
    m_time = data["Time"].toTime();
    m_date = data["Date"].toDate();

    kDebug() << "dataUpdated() was called.";

    if (m_time.minute() == m_lastTimeSeen.minute()) {
        // avoid unnecessary repaints
//         kDebug() << "avoided unnecessary update!";
        return;
    }

    m_lastTimeSeen = m_time;

    calculateDateString();
    calculateTimeString();

    //The timestring changed.
    if( m_timeString != m_lastTimeStringSeen || m_dateString != m_lastDateStringSeen ) {

        //The size might have changed
        calculateSize();

        m_lastTimeStringSeen = m_timeString;
        m_lastDateStringSeen = m_dateString;

        updateGeometry();

        //request to get painted.
        update();
    }
}

void Clock::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->buttons() == Qt::LeftButton && contentRect().contains(event->pos())) {
        showCalendar(event);
    } else {
        event->ignore();
    }
}

void Clock::showCalendar(QGraphicsSceneMouseEvent *event)
{
    if (m_calendar == 0) {
        m_calendar = new Plasma::Dialog();
        //m_calendar->setStyleSheet("{ border : 0px }"); // FIXME: crashes
        m_layout = new QVBoxLayout();
        m_layout->setSpacing(0);
        m_layout->setMargin(0);

        m_calendarUi.setupUi(m_calendar);
        m_calendar->setLayout(m_layout);
        m_calendar->setWindowFlags(Qt::Popup);
        m_calendar->adjustSize();
    }

    if (m_calendar->isVisible()) {
        m_calendar->hide();
    } else {
        m_calendar->position(event, boundingRect(), mapToScene(boundingRect().topLeft()));
        m_calendar->show();
    }
}

void Clock::showConfigurationInterface()
{
    if (m_dialog == 0) {
        m_dialog = new KDialog;
        m_dialog->setCaption( i18n("Configure Fuzzy-Clock") );
        QWidget *widget = new QWidget;
        ui.setupUi(widget);
        m_dialog->setMainWidget(widget);
        m_dialog->setButtons( KDialog::Ok | KDialog::Cancel | KDialog::Apply );
        connect( m_dialog, SIGNAL(applyClicked()), this, SLOT(configAccepted()) );
        connect( m_dialog, SIGNAL(okClicked()), this, SLOT(configAccepted()) );
    }

    ui.useLocalTimeZone->setChecked( m_useLocalTimezone );
    //FIXME: There are several bugs in KTimeZones. 1. Even though the correct zone is selected debug-output states "no such zon: 'nameofZone'"
    //2. If one opens the settings the first time setSelected does not work. It only does the second time settings are opened.
    //3. If the sorting is set to true the widget does not return any values.
    ui.timeZones->setSelected( m_timezone.name(), true );
    ui.timeZones->setEnabled( m_useLocalTimezone );

    ui.fuzzynessSlider->setSliderPosition( m_fuzzyness );
    ui.showTimezone->setChecked( m_showTimezone );
    ui.showDate->setChecked( m_showDate );
    ui.showYear->setChecked( m_showYear );
    ui.showDay->setChecked( m_showDay );
    ui.adjustToHeight->setSliderPosition( m_adjustToHeight );

    ui.fontTimeBold->setChecked(m_fontTimeBold);
    ui.fontTime->setCurrentFont(m_fontTime);
    ui.fontColor->setColor(m_fontColor);

     //Set focus to this widget, oxygen's colour for selected items in a non-focused widgets are not readable.
    ui.timeZones->setFocus( Qt::OtherFocusReason );
    m_dialog->show();
}

void Clock::configAccepted()
{
    KConfigGroup cg = config();
    QGraphicsItem::update();
    QStringList tzs = ui.timeZones->selection();

    m_fontTime = ui.fontTime->currentFont();
    cg.writeEntry("fontTime", m_fontTime);

    //In case adjustToHeight was disabled we have to reset the point-size of fontTime
    m_fontTime.setPointSize ( m_fontDate.pointSize() );

    m_fontColor = ui.fontColor->color();
    cg.writeEntry("fontColor", m_fontColor);

    m_fontTimeBold = ui.fontTimeBold->isChecked();
    cg.writeEntry("fontTimeBold", m_fontTimeBold);

    m_fontTime.setBold(m_fontTimeBold);

    m_fuzzyness = ui.fuzzynessSlider->value();
    cg.writeEntry("fuzzyness", m_fuzzyness);

    //save timezone even though we do not use it to allow the user to switch back easily.
    QString unusedTimezone = m_timezone.name();

    if ( ui.useLocalTimeZone->isChecked() ) { //use local timezone
        dataEngine("time")->disconnectSource(m_timezone.name(), this);

        m_timezone = m_localTimezone;
        dataEngine("time")->connectSource(m_timezone.name(), this);
    } else if ( !tzs.isEmpty() ) {
        QString tz = tzs.at(0);

        if ( tz != m_timezone.name() ) {//A different timezone was selected.
            dataEngine("time")->disconnectSource(m_timezone.name(), this);
            m_timezone = KSystemTimeZones::zone ( tz );
            dataEngine("time")->connectSource(m_timezone.name(), this);
        }
    } else if ( m_timezone.name() != m_localTimezone.name() ) { //No timezone was selected and m_timezone is not the local timezone -> no time would show up.
        dataEngine("time")->disconnectSource(m_timezone.name(), this);
        m_timezone = m_localTimezone;
        dataEngine("time")->connectSource(m_timezone.name(), this);
    }

    if ( ui.useLocalTimeZone->isChecked() ) { //save non-local timezone
        cg.writeEntry("timezone", unusedTimezone);
    } else {
        cg.writeEntry("timezone", m_timezone.name());
    }

    m_useLocalTimezone = ui.useLocalTimeZone->isChecked();
    cg.writeEntry("useLocalTimezone", m_useLocalTimezone);

    m_showDate = ui.showDate->isChecked();
    cg.writeEntry("showDate", m_showDate);
    m_showYear = ui.showYear->isChecked();
    cg.writeEntry("showYear", m_showYear);
    m_showDay = ui.showDay->isChecked();
    cg.writeEntry("showDay", m_showDay);

    m_adjustToHeight = ui.adjustToHeight->value();
    kDebug() << "adjustToHeight" << m_adjustToHeight;

    cg.writeEntry("adjustToHeight", m_adjustToHeight);

    m_showTimezone = ui.showTimezone->isChecked();
    cg.writeEntry("showTimezone", m_showTimezone);

    kDebug() << "New timezone: " << m_timezone.name();

    dataEngine("time")->connectSource(m_timezone.name(), this, 6000, Plasma::AlignToMinute);

    m_configUpdated = true;
    updateConstraints();

    emit configNeedsSaving();
}

Clock::~Clock()
{
}

void Clock::calculateDateString()
{
    if (!m_date.isValid() || ( m_showTimezone == false && m_showDate == false) ) {
            return;
        }

    const QString day = m_date.toString("dd");
    const QString month = m_date.toString("MMM");
    const QString year = m_date.toString("yyyy");

    //Copied from the digital-clock
    if (m_showDate) {
        if (m_showYear) {
            m_dateString = i18nc("@label Short date: "
                                "%1 day in the month, %2 short month name, %3 year",
                                "%1 %2 %3", day, month, year);
        }
        else {
            m_dateString = i18nc("@label Short date: "
                                "%1 day in the month, %2 short month name",
                                "%1 %2", day, month);
        }
        if (m_showDay) {
            QString weekday = QDate::shortDayName(m_date.dayOfWeek());
            m_dateString = i18nc("@label Day of the week with date: "
                                "%1 short day name, %2 short date",
                                "%1, %2", weekday, m_dateString);
        }
    }

//     QString newDateString = m_locale->formatDate ( m_date , m_locale->ShortDate );

//     if( m_showDate == true ) {
//         m_dateString = newDateString;
//     }

    if( m_showTimezone == true ) {
        m_timezoneString = '(' + m_timezone.name() + ')';
    }
}

void Clock::calculateTimeString()
{
    if (!m_time.isValid()) {
        return;
    }

    const int hours = m_time.hour();
// int hours = 1;
    const int minutes = m_time.minute();
// int minutes = 0;

    hourNames   << i18nc("hour in the messages below","one")
                << i18nc("hour in the messages below","two")
                << i18nc("hour in the messages below","three")
                << i18nc("hour in the messages below","four")
                << i18nc("hour in the messages below","five")
                << i18nc("hour in the messages below","six")
                << i18nc("hour in the messages below","seven")
                << i18nc("hour in the messages below","eight")
                << i18nc("hour in the messages below","nine")
                << i18nc("hour in the messages below","ten")
                << i18nc("hour in the messages below","eleven")
                << i18nc("hour in the messages below","twelve");


    normalFuzzy << ki18nc("%1 the hour translated above","%1 o'clock")
                << ki18nc("%1 the hour translated above","five past %1")
                << ki18nc("%1 the hour translated above","ten past %1")
                << ki18nc("%1 the hour translated above","quarter past %1")
                << ki18nc("%1 the hour translated above","twenty past %1")
                << ki18nc("%1 the hour translated above","twenty five past %1")
                << ki18nc("%1 the hour translated above","half past %1")
                << ki18nc("%1 the hour translated above","twenty five to %1")
                << ki18nc("%1 the hour translated above","twenty to %1")
                << ki18nc("%1 the hour translated above","quarter to %1")
                << ki18nc("%1 the hour translated above","ten to %1")
                << ki18nc("%1 the hour translated above","five to %1")
                << ki18nc("%1 the hour translated above","%1 o'clock");

    bool upcaseFirst = i18nc("Whether to uppercase the first letter of "
                             "completed fuzzy time strings above: "
                             "translate as 1 if yes, 0 if no.",
                             "1") != QString('0');

    dayTime << i18n("Night")
            << i18n("Early morning") << i18n("Morning") << i18n("Almost noon")
            << i18n("Noon") << i18n("Afternoon") << i18n("Evening")
            << i18n("Late evening");

    //Create time-string
    QString newTimeString;

    if (m_fuzzyness == 1 || m_fuzzyness == 2) {
        // NOTE: Time strings are deliberately assembled here with English
        // only in mind: translators are able to script the translation to
        // their liking, and this code provides the least surprise for that.
        // Those inquiring should be directed to kde-i18n-doc mailing list
        // for instructions on how to make it right for their language.

        int sector = 0;
        int realHour = 0;

        if (m_fuzzyness == 1) {
            if (minutes > 2) {
                sector = (minutes - 3) / 5 + 1;
            }
        } else {
            if (minutes > 6) {
                sector = ((minutes - 7) / 15 + 1) * 3;
            }
        }

        int deltaHour = (sector <= 6 ? 0 : 1);
        if ((hours + deltaHour) % 12 > 0) { //there is a modulo
            realHour = (hours + deltaHour) % 12 - 1;
        } else {
            realHour = 12 - ((hours + deltaHour) % 12 + 1);
        }

        newTimeString = normalFuzzy[sector].subs(hourNames[realHour]).toString();
        if (upcaseFirst) {
            newTimeString.replace(0, 1, QString(newTimeString.at(0).toUpper()));
        }
    } else if (m_fuzzyness == 3) {
        newTimeString = dayTime[hours / 3];
    } else {
        //Timezones not yet implemented: int dow = QDateTime::currentDateTime().addSecs(TZoffset).date().dayOfWeek();
        int dow = QDateTime::currentDateTime().date().dayOfWeek();

        if (dow == 1) {
            newTimeString = i18n("Start of week");
        }
        else if (dow >= 2 && dow <= 4) {
            newTimeString = i18n("Middle of week");
        }
        else if (dow == 5) {
            newTimeString = i18n("End of week");
        }
        else {
            newTimeString = i18n("Weekend!");
        }
    }

    m_timeString = newTimeString;
}

void Clock::calculateSize()
{
//Minimal sizes.
// QFont minimalFontTime = m_fontTime;
// minimalFontTime.setPointsize ( m_fontDate.pointsize() );
// QFontMetrics fmMinimalTime ( minimalFontTime );
// 
// minimalTimeStringSize = QSizeF( minimalFontTime.width( m_timeString ) + m_margin*2,minimalFontTime.height() );

    //In case adjustToHeight was disabled we have to reset the point-size of fontTime
    m_fontTime.setPointSize ( m_fontDate.pointSize() );

//Actual size, set in config or init
QFontMetrics m_fmTime ( m_fontTime );
m_timeStringSize = QSizeF( m_fmTime.width( m_timeString ) + m_margin*2,m_fmTime.height() );

QFontMetrics m_fmDate ( m_fontDate );

//The date+timezone are currently hardcoded to the smallestReadableFont
m_dateStringSize = QSizeF ( m_fmDate.width( m_dateString ), m_fmDate.height() );
m_timezoneStringSize = QSizeF( m_fmDate.width( m_timezoneString ), m_fmDate.height() );

//If we are in the panel we will use the least possible space for the fontSize set, always!

if ( contentSize().width() > m_timeStringSize.width() && (formFactor() == Plasma::Planar || formFactor() == Plasma::MediaCenter)) { //plasmoid wider than timestring
    kDebug() << "Plasmoid wider than the timestring";
    if( m_showDate == true && m_showTimezone == true ) { //date + timezone enabled
        kDebug() << "Date + Timezone enabled";
        if ( contentSize().width() > m_dateStringSize.width() +  m_timezoneStringSize.width() ) { //date + timezone fit -> 2 rows within the plasmoid

            kDebug() << "plasmoid wider than date + timezone in 1 row";
            m_subtitleString = m_dateString + ' ' + m_timezoneString; //Set subtitleString

            //set subtitleSize
            m_subtitleStringSize = QSizeF ( m_fmDate.width ( m_dateString + ' ' + m_timezoneString ) , m_dateStringSize.height()*1 );
        } else { //date + timezone are split into two lines to fit the plasmoid -> 3 rows within the plasmoid
            kDebug() << "Plasmoid not wide enough for date + timezone in 1 row -> 2 rows";
            m_subtitleString = m_dateString + '\n' + m_timezoneString; //Set subtitleString

            //set subtitleSize
            m_subtitleStringSize = QSizeF ( qMax( m_dateStringSize.width(),m_timezoneStringSize.width() ) , m_dateStringSize.height()*2 );
        }
    } else if ( m_showDate == true ) {
        kDebug() << "Only Date enabled";
        m_subtitleString = m_dateString; //Set subtitleString

        //set subtitleSize
        m_subtitleStringSize = QSizeF ( m_dateStringSize.width() , m_dateStringSize.height() );
    } else if ( m_showTimezone == true ) {
        kDebug() << "Only timezone enabled";
        m_subtitleString = m_timezoneString; //Set subtitleString

        //set subtitleSize
        m_subtitleStringSize = QSizeF ( m_timezoneStringSize.width() , m_timezoneStringSize.height() );
    } else { //no subtitle
        kDebug() << "Neither date nor timezone enabled";
        m_subtitleStringSize = QSizeF ( 0,0 );
    }

//     //If the date/timezone was re-enabled we might have a wide enough, but not high enough plasmoid whose minimumContentSize only fits the timestring. -> increase minimumContentSize
    if ( m_timeStringSize.height() + m_verticalSpacing + m_subtitleStringSize.height() > minimumContentSize().height() ) {
        kDebug() << "Although the plasmoid is wider than necessary the height is too small -> set new minimum height: " << m_timeStringSize.height() + m_verticalSpacing + m_subtitleStringSize.height();
        setMinimumContentSize ( QSizeF ( minimumContentSize().width(),m_timeStringSize.height() + m_verticalSpacing + m_subtitleStringSize.height() ) );

        kDebug() << "New minimumContentSize(): " << minimumContentSize();
    }

    //Make the timestring fit the plasmoid since it is bigger than smallestReadable
    m_fontTime.setPointSize(qMax((int)( contentSize().height()/1.5), 1) );

    m_fmTime = QFontMetrics( m_fontTime );

    while ( ( m_fmTime.width( m_timeString ) > contentSize().width() || m_fmTime.height() > contentSize().height() - m_subtitleStringSize.height() - m_verticalSpacing ) && m_fontTime.pointSize() > 1 ) {

        //decrease pointSize
        m_fontTime.setPointSize(m_fontTime.pointSize() - 1);

        m_fmTime = QFontMetrics( m_fontTime );

        m_timeStringSize = QSizeF ( m_fmTime.width( m_timeString ),  m_fmTime.height() );
    }

    //Adjust the height to the new horizontal size
    m_contentSize = QSizeF ( contentSize().width(),m_timeStringSize.height() + m_verticalSpacing + m_subtitleStringSize.height() );

    if ( m_fixedHeight == true ) { //if we are on the panel we are forced to accept the given height.
        kDebug() << "needed height: " << m_contentSize.height() << "fixed height forced on us: " << contentSize().height();
        setContentSize ( QSizeF ( m_contentSize.width(),contentSize().height() ) );
    } else {
        setContentSize ( QSizeF ( m_contentSize.width(),m_contentSize.height() ) );
    }

} else { //timestring wider than plasmoid -> change size to the minimal needed space, i.e. the timestring will not increase in point-size OR plasmoid in Panel.

    kDebug() << "Plasmoid is too small for timestring, we are using smallestReadable as pointSize";

    if ( m_showDate == true && m_showTimezone == true ) { //Date + timezone enabled
        kDebug() << "Date + timezone enabled";

        //If the user has set adjustToHeight to true the timezone and date are put into one line. This is a design decision and not based on anything else but the opinion that a slightly increased point-size is not what adjustToHeight is meant for. The latter is meant to replace the need to be able to set a font-size in the settings. Bigger fonts than this and only slightly bigger ones don't make sense on the panel. The desktop-plasmoid is not imfluenced by this.
        if( m_timeStringSize.width() > m_dateStringSize.width() +  m_timezoneStringSize.width() || m_adjustToHeight != 0 ) { //Time wider than date + timezone -> 2 rows in plasmoid
            kDebug() << "timestring is wider than date + timezone in one row -> 1 row.";
            m_subtitleString = m_dateString + ' ' + m_timezoneString; //Set subtitleString

            //set subtitleSize
            m_subtitleStringSize = QSizeF ( m_fmDate.width ( m_dateString + ' ' + m_timezoneString ) , m_dateStringSize.height()*1 );

            //set new minimal width to fit the strings
            m_minimumContentSize = QSizeF ( m_timeStringSize.width(),m_subtitleStringSize.height() + m_verticalSpacing + m_subtitleStringSize.height() );
        } else { //date and timezone have to be split.
            kDebug() << "Date + timezone do not fit into one row -> 2 rows.";
            m_subtitleString = m_dateString + '\n' + m_timezoneString; //Set subtitleString

            //set subtitleSize
            m_subtitleStringSize = QSizeF ( qMax ( m_dateStringSize.width(),m_timezoneStringSize.width() ), m_dateStringSize.height()*2 );

            kDebug() << "max: " << qMax ( m_timeStringSize.width(),qMax ( m_dateStringSize.width(),m_timezoneStringSize.width() ) ) << " timestring: " << m_timeStringSize.width() << "date: " << m_dateStringSize.width() << "timezone: " << m_timezoneStringSize.width();

            //set new minimal width to fit widest string and adjust the height
            m_minimumContentSize = QSizeF ( qMax ( m_timeStringSize.width(),qMax ( m_dateStringSize.width(),m_timezoneStringSize.width() ) ),m_timeStringSize.height() + m_verticalSpacing + m_subtitleStringSize.height() );
        }
    } else if ( m_showDate == true ) {
        kDebug() << "Only date is enabled";
        m_subtitleString = m_dateString; //Set subtitleString

        //set subtitleSize
        m_subtitleStringSize = QSizeF ( m_dateStringSize.width(), m_dateStringSize.height() );

        //set new minimal width to fit the widest string
        m_minimumContentSize = QSizeF ( qMax ( m_dateStringSize.width(),m_timeStringSize.width() ),m_timeStringSize.height() + m_verticalSpacing + m_subtitleStringSize.height() );
    } else if ( m_showTimezone == true ) {
        kDebug() << "Only timezone is enabled";
        m_subtitleString = m_timezoneString; //Set subtitleString

        //set subtitleSize
        m_subtitleStringSize = QSizeF ( m_timezoneStringSize.width(), m_timezoneStringSize.height() );

        //set new size to fit the strings
        m_minimumContentSize = QSizeF ( qMax ( m_timezoneStringSize.width(),m_timeStringSize.width() ),m_timeStringSize.height() + m_verticalSpacing + m_subtitleStringSize.height() );
    } else { //no subtitle
        kDebug() << "Neither timezone nor date are enabled";
        //set subtitleSize
        m_subtitleStringSize = QSizeF ( 0,0 );

        m_minimumContentSize = QSizeF ( m_timeStringSize.width(),m_timeStringSize.height() );//set new minimal width
    }

    float heightToUse = 0;

    //Use x of the available height for the timstring
    if ( m_adjustToHeight == 1 ) {
        heightToUse = (float)2/3;
        kDebug() << "We will use 2/3 of the panel's height for the time: " << heightToUse;
    } else if ( m_adjustToHeight == 2 ) {
        kDebug() << "We will use all of the panel's height for the time.";
        heightToUse = 1;
    }

    //If the user enabled adjustToHeight we increase the point-size until the height is fully used. 40 as limit to avoid an endless loop.
    if ( m_adjustToHeight != 0 ) {
        while ( ( m_fmTime.height() < ( contentSize().height() - m_subtitleStringSize.height() - m_verticalSpacing )*heightToUse ) && m_fontTime.pointSize() < 40 ) {
    
            //decrease pointSize
            m_fontTime.setPointSize(m_fontTime.pointSize() + 1);
    
            m_fmTime = QFontMetrics( m_fontTime );
    
            m_timeStringSize = QSizeF ( m_fmTime.width( m_timeString ),  m_fmTime.height() );
        }
    
        //Adjust the width to the new size
        m_minimumContentSize = QSizeF ( m_timeStringSize.width() + m_margin*2,m_minimumContentSize.height() );
    }

    kDebug() << "Set new minimumContentSize. contentSize() " << contentSize() << "minimumSize: " << m_minimumContentSize;

    if ( m_fixedHeight == true ) { //if we are on the panel we are forced to accept the given height.
        kDebug() << "needed height: " << m_minimumContentSize.height() << "fixed height forced on us: " << contentSize().height() << " adding margin left/right of: " << m_margin;

        //If the minimal size does not fit the minimal font
        if( m_fontTime.pointSize() <= m_fontDate.pointSize() ) {
            //set new minimumSize
            setMinimumContentSize ( QSizeF ( m_minimumContentSize.width() + m_margin*2,m_minimumContentSize.height() ) );
        }
        setContentSize ( QSizeF ( m_minimumContentSize.width() + m_margin*2,contentSize().height() ) );
    } else { //FIXME: In case this height does not fit the content -> disable timezone (and date)
        if( m_fontTime.pointSize() <= m_fontDate.pointSize() ) {
            //set new minimumSize
            setMinimumContentSize ( QSizeF ( m_minimumContentSize.width() + m_margin*2,m_minimumContentSize.height() ) );
        }
        setContentSize ( QSizeF ( m_minimumContentSize.width() + m_margin*2,contentSize().height() ) );
    }
}
}

void Clock::paintInterface(QPainter *p, const QStyleOptionGraphicsItem *option, const QRect &contentsRect)
{
    Q_UNUSED( option );
    Q_UNUSED( contentsRect );

    kDebug() << "We get painted!";

    if( m_showDate == true || m_showTimezone == true ) {

        m_fontDate = QFont( KGlobalSettings::smallestReadableFont() );
        QFontMetrics m_fmDate( m_fontDate );
        p->setPen(QPen(m_fontColor));
        p->setFont( m_fontDate );

        kDebug() << "date + timezone [" << m_subtitleString << "] gets painted. y: " << -m_subtitleStringSize.height() + contentSize().height() << "width: " << contentSize().width() << "[needed: " << m_fmDate.width( m_subtitleString ) << "] " << "height:" << m_subtitleStringSize.height();

        if( m_showDate == true || m_showTimezone == true ) {
        //Draw the subtitle
        p->drawText( QRectF(0,
                -m_subtitleStringSize.height() + contentSize().height(),
                contentSize().width(),
                m_subtitleStringSize.height()) ,
            m_subtitleString,
            QTextOption(Qt::AlignHCenter)
            );
        }
    }

        QFontMetrics m_fmTime ( m_fontTime );

        kDebug() << "timestrings [" << m_timeString << "] gets painted. width: " << contentSize().width() << "[needed: " << m_fmTime.width( m_timeString ) << "] " << "height: " << m_timeStringSize.height();

        p->setFont( m_fontTime );
        p->setPen(QPen(m_fontColor));
        p->setRenderHint(QPainter::SmoothPixmapTransform);
        p->setRenderHint(QPainter::Antialiasing);

        p->drawText( QRectF(0,
                0,
                contentSize().width(),
                m_timeStringSize.height()) ,
            m_timeString,
            QTextOption(Qt::AlignHCenter)
            );
}

#include "fuzzyClock.moc"
