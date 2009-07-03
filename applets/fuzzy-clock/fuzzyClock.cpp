/***************************************************************************
 *   Copyright (c) 1996-2002 the kicker authors. (fuzzy logic)             *
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

#include <QFontMetrics>
#include <QPainter>
#include <QFontInfo>

#include <KColorScheme>
#include <KConfigDialog>
#include <KDebug>

#include <Plasma/Theme>

Clock::Clock(QObject *parent, const QVariantList &args)
    : ClockApplet(parent, args),
      m_oldContentSize(QSizeF (0,0)),
      m_adjustToHeight(1),
      m_useCustomFontColor(false),
      m_fontColor(Qt::white),
      m_fontTimeBold(false),
      m_fontTimeItalic(false),
      m_fontTime(KGlobalSettings::smallestReadableFont()),
      m_showTimezone(false),
      m_showDate(false),
      m_showYear(false),
      m_showDay(false),
      m_layout(0)
{
    KGlobal::locale()->insertCatalog("libplasmaclock");
    KGlobal::locale()->insertCatalog("timezones4");
    setHasConfigurationInterface(true);
    setBackgroundHints(Plasma::Applet::DefaultBackground);
    //If we do not set this, the user has to press CTRL when shrinking the plasmoid on the desktop beyond a certain size.
    setAspectRatioMode(Plasma::IgnoreAspectRatio);
}

Clock::~Clock()
{
}

void Clock::init()
{
    ClockApplet::init();

    initFuzzyTimeStrings();

    m_contentSize = geometry().size();

    kDebug() << "The first content's size [geometry().size()] we get, init() called: " << geometry().size();

    m_locale = KGlobal::locale();

    KConfigGroup cg = config();

    m_showTimezone = cg.readEntry("showTimezone", false);
    m_showDate = cg.readEntry("showDate", true);
    m_showYear = cg.readEntry("showYear",false);
    m_showDay = cg.readEntry("showDay",true);

    m_fuzzyness = cg.readEntry("fuzzyness", 1);

    m_fontTime = cg.readEntry("fontTime", KGlobalSettings::smallestReadableFont());
    m_useCustomFontColor = cg.readEntry("useCustomFontColor", false);
    if (m_useCustomFontColor){
        m_fontColor = cg.readEntry("fontColor", m_fontColor);
    }else{
        m_fontColor = KColorScheme(QPalette::Active, KColorScheme::View, Plasma::Theme::defaultTheme()->colorScheme()).foreground().color();
    }
    m_fontTimeBold = cg.readEntry("fontTimeBold", true);
    m_fontTimeItalic = cg.readEntry("fontTimeItalic", false);

    m_fontTime.setBold(m_fontTimeBold);
    m_fontTime.setItalic(m_fontTimeItalic);

    m_adjustToHeight = cg.readEntry("adjustToHeight", 1);

    //By default we use the smallest readable font.
    m_fontDate = QFont ( KGlobalSettings::smallestReadableFont() );

    m_margin = 2;
    m_verticalSpacing = 2;

    Plasma::DataEngine* timeEngine = dataEngine("time");
    timeEngine->connectSource(currentTimezone(), this, 6000, Plasma::AlignToMinute);

    connect(Plasma::Theme::defaultTheme(), SIGNAL(themeChanged()), this, SLOT(updateColors()));
}

Qt::Orientations Clock::expandingDirections() const
{
    //This tells the layout whether it's ok to be stretched, even if we do not need that space. Since we would become far too wide on a panel we do not want that.
    return 0;
}

// QSizeF Clock::contentSizeHint() const
// {
//     return contentSize();
// }

void Clock::constraintsEvent(Plasma::Constraints constraints)
{
    kDebug() << "constraintsEvent() called";

    if (constraints & Plasma::SizeConstraint || constraints & Plasma::FormFactorConstraint) {
        if ( (m_oldContentSize.toSize() != geometry().size() && m_oldContentSize.toSize() != QSize (0,0)) || m_configUpdated == true ) { //The size changed or config was updated
            kDebug() << "The content's size [geometry().size()] changed! old: " << m_oldContentSize << "new: " << geometry().size();

            if ( m_configUpdated ) {
                calculateDateString();
                calculateTimeString();
            }

            kDebug() << "Constraints changed: " << constraints;

            if (formFactor() == Plasma::Planar || formFactor() == Plasma::MediaCenter) {
                kDebug() << "######## Other FormFactor";

                calculateSize();
            } else {
                kDebug() << "######## Small FormFactor";

                calculateSize();
            }

            kDebug() << "The new size has been calculated and set.\nneeded m_contenSize (if not in panel): " << m_contentSize << "\nactual content's size [geometry().size()] is: " << geometry().size() << "\nminimumSize() needed (in panel): " << minimumSize();

            m_oldContentSize = geometry().size();
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

    if (Plasma::ToolTipManager::self()->isVisible(this)) {
        updateTipContent();
    }

    m_lastTimeSeen = m_time;

    calculateDateString();
    calculateTimeString();

    //The timestring changed.
    if (m_timeString != m_lastTimeStringSeen || m_dateString != m_lastDateStringSeen) {

        //The size might have changed
        calculateSize();

        m_lastTimeStringSeen = m_timeString;
        m_lastDateStringSeen = m_dateString;

        updateGeometry();

        //request to get painted.
        update();
    }
}

void Clock::createClockConfigurationInterface(KConfigDialog *parent)
{
    QWidget *widget = new QWidget();
    ui.setupUi(widget);
    parent->addPage(widget, i18n("General"), icon());

    ui.fuzzynessSlider->setSliderPosition( m_fuzzyness );
    ui.showTimezone->setChecked( m_showTimezone );
    ui.showDate->setChecked( m_showDate );
    ui.showYear->setChecked( m_showYear );
    ui.showDay->setChecked( m_showDay );
    ui.adjustToHeight->setSliderPosition( m_adjustToHeight );

    ui.fontTimeBold->setChecked(m_fontTimeBold);
    ui.fontTimeItalic->setChecked(m_fontTimeItalic);
    ui.fontTime->setCurrentFont(m_fontTime);
    ui.fontColor->setColor(m_fontColor);
    ui.useCustomFontColor->setChecked(m_useCustomFontColor);
}

void Clock::clockConfigAccepted()
{
    KConfigGroup cg = config();
    QGraphicsItem::update();

    m_fontTime = ui.fontTime->currentFont();
    cg.writeEntry("fontTime", m_fontTime);

    //In case adjustToHeight was disabled we have to reset the point-size of fontTime
    m_fontTime.setPointSize ( m_fontDate.pointSize() );

    m_useCustomFontColor = ui.useCustomFontColor->isChecked();
    cg.writeEntry("useCustomFontColor", m_useCustomFontColor);
    if (m_useCustomFontColor) {
        m_fontColor = ui.fontColor->color();
    } else {
        m_fontColor = KColorScheme(QPalette::Active, KColorScheme::View, Plasma::Theme::defaultTheme()->colorScheme()).foreground().color();
    }
    cg.writeEntry("fontColor", ui.fontColor->color());

    m_fontTimeBold = ui.fontTimeBold->isChecked();
    cg.writeEntry("fontTimeBold", m_fontTimeBold);

    m_fontTimeItalic = ui.fontTimeItalic->isChecked();
    cg.writeEntry("fontTimeItalic", m_fontTimeItalic);

    m_fontTime.setBold(m_fontTimeBold);
    m_fontTime.setItalic(m_fontTimeItalic);

    m_fuzzyness = ui.fuzzynessSlider->value();
    cg.writeEntry("fuzzyness", m_fuzzyness);

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

    dataEngine("time")->connectSource(currentTimezone(), this, 6000, Plasma::AlignToMinute);

    m_configUpdated = true;
    updateConstraints();

    emit configNeedsSaving();
}

void Clock::changeEngineTimezone(const QString &oldTimezone, const QString &newTimezone)
{
    dataEngine("time")->disconnectSource(oldTimezone, this);
    dataEngine("time")->connectSource(newTimezone, this, 6000, Plasma::AlignToMinute);
}

void Clock::calculateDateString()
{
    if (!m_date.isValid() || ( m_showTimezone == false && m_showDate == false) ) {
            return;
        }

    KLocale tmpLocale(*KGlobal::locale());
    tmpLocale.setDateFormat("%e"); // day number of the month
    QString day = tmpLocale.formatDate(m_date);
    tmpLocale.setDateFormat("%b"); // short form of the month
    QString month = tmpLocale.formatDate(m_date);
    tmpLocale.setDateFormat("%Y"); // the year with four digits
    QString year = tmpLocale.formatDate(m_date);

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
            tmpLocale.setDateFormat("%a"); // short weekday
            QString weekday = tmpLocale.formatDate(m_date);
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
        QString timezonetranslated = i18n( currentTimezone().toUtf8().data());
        timezonetranslated = timezonetranslated.replace('_', ' ');
        m_timezoneString = '(' + timezonetranslated  + ')';
    }
}

void Clock::initFuzzyTimeStrings()
{
    m_hourNames   << i18nc("hour in the messages below","one")
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

    m_normalFuzzy << ki18nc("%1 the hour translated above","%1 o'clock")
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

    m_dayTime << i18n("Night")
            << i18n("Early morning") << i18n("Morning") << i18n("Almost noon")
            << i18n("Noon") << i18n("Afternoon") << i18n("Evening")
            << i18n("Late evening");

    m_weekTime << i18n("Start of week")
               << i18n("Middle of week")
               << i18n("End of week")
               << i18n("Weekend!");
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

    bool upcaseFirst = i18nc("Whether to uppercase the first letter of "
                             "completed fuzzy time strings above: "
                             "translate as 1 if yes, 0 if no.",
                             "1") != QString('0');

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

        newTimeString = m_normalFuzzy[sector].subs(m_hourNames[realHour]).toString();
        if (upcaseFirst) {
            newTimeString.replace(0, 1, QString(newTimeString.at(0).toUpper()));
        }
    } else if (m_fuzzyness == 3) {
        newTimeString = m_dayTime[hours / 3];
    } else {
        //Timezones not yet implemented: int dow = QDateTime::currentDateTime().addSecs(TZoffset).date().dayOfWeek();
        int dow = QDateTime::currentDateTime().date().dayOfWeek();

        int weekStrIdx;
        if (dow == 1) {
            weekStrIdx = 0;
        }
        else if (dow >= 2 && dow <= 4) {
            weekStrIdx = 1;
        }
        else if (dow == 5) {
            weekStrIdx = 2;
        }
        else {
            weekStrIdx = 3;
        }

        newTimeString = m_weekTime[weekStrIdx];
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

int minimumWantedSize = KGlobalSettings::smallestReadableFont().pointSize();
if (formFactor() == Plasma::Horizontal) {
    QFont font(m_fontTime);
    font.setPixelSize(size().height()/2);
    QFontInfo fi(font);
    minimumWantedSize = fi.pointSize();
}


if ( contentsRect().size().width() > m_timeStringSize.width() && (formFactor() == Plasma::Planar || formFactor() == Plasma::MediaCenter)) { //plasmoid wider than timestring
    kDebug() << "Plasmoid wider than the timestring";
    if( m_showDate == true && m_showTimezone == true ) { //date + timezone enabled
        kDebug() << "Date + Timezone enabled";
        if ( contentsRect().size().width() > m_dateStringSize.width() +  m_timezoneStringSize.width() ) { //date + timezone fit -> 2 rows within the plasmoid

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
    if ( m_timeStringSize.height() + m_verticalSpacing + m_subtitleStringSize.height() > minimumSize().height() ) {
        kDebug() << "Although the plasmoid is wider than necessary the height is too small -> set new minimum height: " << m_timeStringSize.height() + m_verticalSpacing + m_subtitleStringSize.height();
        setMinimumSize ( QSizeF ( minimumSize().width(),m_timeStringSize.height() + m_verticalSpacing + m_subtitleStringSize.height() ) );

        kDebug() << "New minimumContentSize(): " << minimumSize();
    }

    //Make the timestring fit the plasmoid since it is bigger than minimumWantedSize
    m_fontTime.setPointSize(qMax((int)( geometry().size().height()/1.5), minimumWantedSize) );

    m_fmTime = QFontMetrics( m_fontTime );

    while ( ( m_fmTime.width( m_timeString ) > contentsRect().size().width() - 2*m_margin ||
              m_fmTime.height() > contentsRect().size().height() - m_subtitleStringSize.height() - m_verticalSpacing ) &&
              m_fontTime.pointSize() > minimumWantedSize) {

        //decrease pointSize
        m_fontTime.setPointSize(m_fontTime.pointSize() - 1);

        m_fmTime = QFontMetrics( m_fontTime );

        m_timeStringSize = QSizeF ( m_fmTime.width( m_timeString ),  m_fmTime.height() );
    }

    //Adjust the height to the new horizontal size
    m_contentSize = QSizeF ( contentsRect().width(),m_timeStringSize.height() + m_verticalSpacing + m_subtitleStringSize.height() );

    if ( formFactor() == Plasma::Horizontal ) { //if we are on the panel we are forced to accept the given height.
        kDebug() << "needed height: " << m_contentSize.height() << "fixed height forced on us: " << geometry().size().height();
        //FIXME: it was resizing to size() itself
        //resize ( QSizeF ( m_contentSize.width(),geometry().size().height() ) );
    } else {
        //add margins
        resize ( m_contentSize + QSizeF(size()-contentsRect().size()) );
        emit appletTransformedItself();
    }

} else { //in a panel or timestring wider than plasmoid -> change size to the minimal needed space, i.e. the timestring will not increase in point-size OR plasmoid in Panel.

    kDebug() << "Plasmoid is in a panel or too small for the timestring, we are using minimumWantedSize as pointSize";

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
        kDebug() << "We try to find a larger font that fits the size:";

        //FIXME: if the clock is the only applet on a vertical panel and returns 0 via expandingDirections(), it still gets the full height of the panel as recommended height, i.e. on a vertical panel width a height of 800, geometry().size().height() does not return 48 but some huge value. Unless this is fixed in plasma, the while-loop will take a while.

        //Make the timestring fit the plasmoid since it is bigger than minimumWantedSize
        m_fontTime.setPointSize(qMax((int)( geometry().size().height()/1.5), minimumWantedSize) );
    
        m_fmTime = QFontMetrics( m_fontTime );
    
        kDebug() << "Starting with a point size of: " << m_fontTime.pointSize();

        kDebug() << "We want to have: \nwidth: < " << geometry().size().width() - 2*m_margin << "\nheight < " << (geometry().size().height() - m_subtitleStringSize.height() - m_verticalSpacing)*heightToUse;

        while ( ( ( m_fmTime.width( m_timeString ) > geometry().size().width() - 2*m_margin && formFactor() != Plasma::Horizontal ) ||
                 m_fmTime.height() > (geometry().size().height() - m_subtitleStringSize.height() - m_verticalSpacing)*heightToUse ) &&
                 m_fontTime.pointSize() > minimumWantedSize) {
    
            //decrease pointSize
            m_fontTime.setPointSize(m_fontTime.pointSize() - 1);

            kDebug() << "new point size: " << m_fontTime.pointSize();
    
            m_fmTime = QFontMetrics( m_fontTime );
    
            m_timeStringSize = QSizeF ( m_fmTime.width( m_timeString ),  m_fmTime.height() );
        }
    }

    //Adjust the width to the new size, including margins, will be reverted, if the panel is vertical
    m_minimumContentSize = QSizeF ( m_timeStringSize.width() + m_margin*2,m_minimumContentSize.height() );

    kDebug() << "Set new minimumSize: geometry().size() " << geometry().size() << "\nm_minimumContentSize: " << m_minimumContentSize;

    //if the width given by the panel is too wide, e.g. when switching from panel at the right to panel at the bottom we get some 600 as width
    //However: If we are in a vertical panel, we should use the width given.
    if( m_timeStringSize.width() + m_margin*2 < geometry().size().width() && formFactor() != Plasma::Vertical ) {
            kDebug() << "The width we got was too big, we need less, so lets resize.";
            setMinimumSize ( m_minimumContentSize + (size() - contentsRect().size()) );
        }

    if ( formFactor() == Plasma::Horizontal ) { //if we are on the panel we are forced to accept the given height.
        kDebug() << "needed height: " << m_minimumContentSize.height() << "[horizontal panel] fixed height forced on us: " << geometry().size().height() << " adding margin-left/-right of: " << m_margin << "width is going to be set resize( " << m_minimumContentSize.width() << "," << geometry().size().height() << ")";

        setMinimumSize(QSizeF(m_minimumContentSize.width(), 0));
        //Expand the panel as necessary
        setPreferredSize(minimumSize());
        emit sizeHintChanged(Qt::PreferredSize);
    } else if ( formFactor() == Plasma::Vertical ) {
        kDebug() << "needed width: " << m_minimumContentSize.width() << "[vertical panel] fixed width forced on us: " << geometry().size().width() << " adding margin left/right of: " << m_margin;

        setMinimumSize ( QSizeF(0, m_minimumContentSize.height()) );
        //Expand the panel as necessary
        setPreferredSize(minimumSize());
        emit sizeHintChanged(Qt::PreferredSize);
    }else { //FIXME: In case this height does not fit the content -> disable timezone (and date)
        //if the minimal width is larger than the actual size -> force minimal needed width
        if( m_fontTime.pointSize() <= m_fontDate.pointSize() ) {
            setMinimumSize ( m_minimumContentSize + (size() - contentsRect().size()) );
        }

        //we use the minimal height here, since the user has given us too much height we cannot use for anything useful. minimal width because we are in a panel.
        kDebug() << "we set the minimum size needed as the size we want";
        resize ( QSizeF ( m_minimumContentSize.width() + m_margin*2,m_minimumContentSize.height() ) + (size() - contentsRect().size()) );
        emit appletTransformedItself();
    }
}
}

void Clock::paintInterface(QPainter *p, const QStyleOptionGraphicsItem *option, const QRect &contentsRect)
{
    Q_UNUSED( option );

    kDebug() << "We get painted!";

    if( m_showDate == true || m_showTimezone == true ) {

        m_fontDate = QFont( KGlobalSettings::smallestReadableFont() );
        QFontMetrics m_fmDate( m_fontDate );
        p->setPen(QPen(m_fontColor));
        p->setFont( m_fontDate );

        kDebug() << "date + timezone [" << m_subtitleString << "] gets painted. y: " << -m_subtitleStringSize.height() + contentsRect.size().height() << "width: " << contentsRect.size().width() << "[needed: " << m_fmDate.width( m_subtitleString ) << "] " << "height:" << m_subtitleStringSize.height();

        if( m_showDate == true || m_showTimezone == true ) {
        //Draw the subtitle
        p->drawText( QRectF(contentsRect.x(),
                contentsRect.y() - m_subtitleStringSize.height() + contentsRect.size().height(),
                contentsRect.size().width(),
                m_subtitleStringSize.height()) ,
            m_subtitleString,
            QTextOption(Qt::AlignHCenter)
            );
        }
    }

        QFontMetrics m_fmTime ( m_fontTime );

        kDebug() << "timestrings [" << m_timeString << "] gets painted. width: " << contentsRect.size().width() << "[needed: " << m_fmTime.width( m_timeString ) << "] " << "height: " << m_timeStringSize.height();

        p->setFont( m_fontTime );
        p->setPen(QPen(m_fontColor));
        p->setRenderHint(QPainter::SmoothPixmapTransform);
        p->setRenderHint(QPainter::Antialiasing);

        p->drawText( QRectF(contentsRect.x(),
                contentsRect.y(),
                contentsRect.size().width(),
                m_timeStringSize.height()) ,
            m_timeString,
            QTextOption(Qt::AlignHCenter)
            );
}

void Clock::updateColors()
{
    if (!m_useCustomFontColor) {
        m_fontColor = KColorScheme(QPalette::Active, KColorScheme::View, Plasma::Theme::defaultTheme()->colorScheme()).foreground().color();
        update();
    }
}

#include "fuzzyClock.moc"
