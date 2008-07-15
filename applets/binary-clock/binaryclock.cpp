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

#include <QPainter>

#include <KConfigDialog>

#include <Plasma/DataEngine>
#include <Plasma/Theme>

#include "ui_clockConfig.h"
#include "binaryclock.h"

BinaryClock::BinaryClock(QObject *parent, const QVariantList &args)
    : ClockApplet(parent, args),
      m_showSeconds(true),
      m_showOffLeds(true),
      m_showGrid(true),
      m_time(0, 0)
{
    KGlobal::locale()->insertCatalog("libplasmaclock");

    setHasConfigurationInterface(true);
    resize(getWidthFromHeight(128), 128);
}

void BinaryClock::init()
{
    KConfigGroup cg = config();
    m_showSeconds = cg.readEntry("showSeconds", m_showSeconds);
    m_showGrid = cg.readEntry("showGrid", m_showGrid);
    m_showOffLeds = cg.readEntry("showOffLeds", m_showOffLeds);
    setCurrentTimezone(cg.readEntry("timezone", localTimezone()));

    connect(Plasma::Theme::defaultTheme(), SIGNAL(themeChanged()), SLOT(updateColors()));

    connectToEngine();

    updateColors();
}

BinaryClock::~BinaryClock()
{
}

int BinaryClock::getHeightFromWidth(int w) const
{
    int dots = m_showSeconds ? 6 : 4;
    int rectSize = (w - 5) * 4;

    return (rectSize / dots) + 3;
}

int BinaryClock::getWidthFromHeight(int h) const
{
    int dots = m_showSeconds ? 6 : 4;
    int rectSize = (h - 3) / 4;

    return (rectSize * dots) + (dots - 1);
}

void BinaryClock::constraintsEvent(Plasma::Constraints constraints)
{
    if (constraints & Plasma::FormFactorConstraint) {
        qreal top, bottom, left, right;
        getContentsMargins(&left, &top, &right, &bottom);
        qreal borderHeight = top + bottom;
        qreal borderWidth = left + right;
        if (formFactor() == Plasma::Vertical) {
            setMaximumHeight(getHeightFromWidth(contentsRect().width()) + borderHeight);
        } else if (formFactor() == Plasma::Horizontal) {
            setMaximumWidth(getWidthFromHeight(contentsRect().height()) + borderWidth);
        } else {
            resize(getWidthFromHeight(contentsRect().height()) + borderWidth, contentsRect().height() + borderHeight);
        }
    }
}

void BinaryClock::connectToEngine()
{
    Plasma::DataEngine* timeEngine = dataEngine("time");

    if (m_showSeconds) {
        timeEngine->connectSource(currentTimezone(), this, 500);
    } else {
        timeEngine->connectSource(currentTimezone(), this, 6000, Plasma::AlignToMinute);
    }
}

void BinaryClock::dataUpdated(const QString& source, const Plasma::DataEngine::Data &data)
{
    Q_UNUSED(source);

    m_time = data["Time"].toTime();

    if (m_time.minute() == m_lastTimeSeen.minute() &&
        m_time.second() == m_lastTimeSeen.second()) {
        // avoid unnecessary repaints
        return;
    }

    m_lastTimeSeen = m_time;

    update();
}

void BinaryClock::createClockConfigurationInterface(KConfigDialog *parent)
{
    QWidget *widget = new QWidget();
    ui.setupUi(widget);
    parent->setButtons( KDialog::Ok | KDialog::Cancel | KDialog::Apply );
    parent->addPage(widget, parent->windowTitle(), icon());

    ui.timeZones->setSelected(currentTimezone(), true);
    ui.timeZones->setEnabled(currentTimezone() != "Local");
    ui.localTimeZone->setChecked(currentTimezone() == "Local");
    ui.showSecondHandCheckBox->setChecked(m_showSeconds);
    ui.showGridCheckBox->setChecked(m_showGrid);
    ui.showOffLedsCheckBox->setChecked(m_showOffLeds);
}

void BinaryClock::clockConfigAccepted()
{
    KConfigGroup cg = config();
    m_showSeconds = ui.showSecondHandCheckBox->isChecked();
    m_showGrid = ui.showGridCheckBox->isChecked();
    m_showOffLeds = ui.showOffLedsCheckBox->isChecked();

    cg.writeEntry("showSeconds", m_showSeconds);
    cg.writeEntry("showGrid", m_showGrid);
    cg.writeEntry("showOffLeds", m_showOffLeds);

    connectToEngine();
    constraintsEvent(Plasma::AllConstraints);
    update();
    emit configNeedsSaving();
}


void BinaryClock::updateColors()
{
    m_ledsColor = QColor(Plasma::Theme::defaultTheme()->color(Plasma::Theme::TextColor));
    m_offLedsColor = QColor(m_ledsColor);
    m_offLedsColor.setAlpha(40);
    m_gridColor = QColor(m_ledsColor);
    m_gridColor.setAlpha(60);

    update();
}

void BinaryClock::paintInterface(QPainter *p, const QStyleOptionGraphicsItem *option,
                                 const QRect &contentsRect)
{
    Q_UNUSED(option);

    if (! m_time.isValid()) {
        return;
    }

    QSizeF m_size = contentsRect.size();
    int appletHeight = (int) contentsRect.height();
    int appletWidth = (int) contentsRect.width();
    int dots = m_showSeconds ? 6 : 4;

    int rectSize = qMax(1, (appletHeight - 3) / 4);
    int yPos = ((appletHeight % rectSize) / 2) + contentsRect.topLeft().y();
    int xPos = ((appletWidth - (rectSize * dots) - 5) / 2) + contentsRect.topLeft().x();

    const QString hours = m_time.toString("HH");
    const QString minutes = m_time.toString("mm");
    const QString seconds = m_time.toString("ss");

    char timeDigits[6] = {hours[0].toAscii(), hours[1].toAscii(),
                          minutes[0].toAscii(), minutes[1].toAscii(),
                          seconds[0].toAscii(), seconds[1].toAscii()};

    for (int i = 0; i < dots; i++) {
        for (int j = 0; j < 4; j++) {
            if (timeDigits[i] & (1 << (3 - j))) {
                p->fillRect(xPos + (i * (rectSize + 1)), yPos + (j * (rectSize + 1)), rectSize, rectSize, m_ledsColor);
            } else if (m_showOffLeds) {
                p->fillRect(xPos + (i * (rectSize + 1)), yPos + (j * (rectSize + 1)), rectSize, rectSize, m_offLedsColor);
            }
        }
    }

    if (m_showGrid) {
        p->setPen(m_gridColor);
        p->drawRect((xPos - 1), (yPos - 1),
                    (dots * (rectSize + 1)), (4 * (rectSize + 1)) );

        for (int i = 1; i < dots; i++) {
            for (int j = 0; j < 4; j++) {
               p->drawLine((xPos + (i * (rectSize + 1)) - 1), (yPos + (j * (rectSize + 1))),
                           (xPos + (i * (rectSize + 1)) - 1), (yPos + (j * (rectSize + 1)) + rectSize - 1) );
            }
        }

        for (int j = 1; j < 4; j++) {
            p->drawLine(xPos, (yPos + (j * (rectSize + 1)) - 1),
                        (xPos + (dots * (rectSize + 1)) - 2), (yPos + (j * (rectSize + 1)) - 1) );
        }
    }
}

#include "binaryclock.moc"
