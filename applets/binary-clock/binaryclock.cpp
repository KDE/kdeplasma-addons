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

#include <KDialog>
#include <Plasma/DataEngine>

#include "ui_clockConfig.h"
#include "binaryclock.h"

BinaryClock::BinaryClock(QObject *parent, const QVariantList &args)
    : Plasma::Applet(parent, args),
    m_ledsColor(Qt::white),
    m_offLedsColor(QColor::fromRgb(255, 255, 255, 40)),
    m_gridColor(QColor::fromRgb(255, 255, 255, 60)),
    m_dialog(0)
{
    setHasConfigurationInterface(true);
    resize(getWidthFromHeight(128), 128);
}

void BinaryClock::init()
{
    KConfigGroup cg = config();
    m_timezone = cg.readEntry("timezone", "Local");
    m_showSeconds = cg.readEntry("showSeconds", true);
    m_showGrid = cg.readEntry("showGrid", true);
    m_showOffLeds = cg.readEntry("showOffLeds", true);

    connectToEngine();
}

BinaryClock::~BinaryClock()
{
}

Qt::Orientations BinaryClock::expandingDirections() const
{
    return 0;
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

    return (rectSize * dots) + 5;
}

QSizeF BinaryClock::contentSizeHint() const
{
    QSizeF sizeHint = geometry().size();

    switch (formFactor()) {
        case Plasma::Vertical:
            sizeHint.setHeight(getHeightFromWidth((int) sizeHint.width()));
            break;

        case Plasma::Horizontal:
            sizeHint.setWidth(getWidthFromHeight((int) sizeHint.height()));
            break;

        default:
            sizeHint.setWidth(getWidthFromHeight((int) sizeHint.height()));
            break;
    }

    return sizeHint;
}

void BinaryClock::connectToEngine()
{
    Plasma::DataEngine* timeEngine = dataEngine("time");

    if (m_showSeconds) {
        timeEngine->connectSource(m_timezone, this, 500);
    } else {
        timeEngine->connectSource(m_timezone, this, 6000, Plasma::AlignToMinute);
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

void BinaryClock::showConfigurationInterface()
{
     if (m_dialog == 0) {
        m_dialog = new KDialog;
        m_dialog->setCaption( i18nc("@title:window","Configure Clock") );

        ui.setupUi(m_dialog->mainWidget());
        m_dialog->mainWidget()->layout()->setMargin(0);
        m_dialog->setButtons( KDialog::Ok | KDialog::Cancel | KDialog::Apply );
        connect( m_dialog, SIGNAL(applyClicked()), this, SLOT(configAccepted()) );
        connect( m_dialog, SIGNAL(okClicked()), this, SLOT(configAccepted()) );
    }

    ui.timeZones->setSelected(m_timezone, true);
    ui.timeZones->setEnabled(m_timezone != "Local");
    ui.localTimeZone->setChecked(m_timezone == "Local");
    ui.showSecondHandCheckBox->setChecked(m_showSeconds);
    ui.showGridCheckBox->setChecked(m_showGrid);
    ui.showOffLedsCheckBox->setChecked(m_showOffLeds);

    m_dialog->show();
}

void BinaryClock::configAccepted()
{
    KConfigGroup cg = config();
    m_showSeconds = ui.showSecondHandCheckBox->isChecked();
    m_showGrid = ui.showGridCheckBox->isChecked();
    m_showOffLeds = ui.showOffLedsCheckBox->isChecked();

    cg.writeEntry("showSeconds", m_showSeconds);
    cg.writeEntry("showGrid", m_showGrid);
    cg.writeEntry("showOffLeds", m_showOffLeds);

    update();
    QStringList tzs = ui.timeZones->selection();

    if (ui.localTimeZone->checkState() == Qt::Checked) {
        dataEngine("time")->disconnectSource(m_timezone, this);
        m_timezone = "Local";
        cg.writeEntry("timezone", m_timezone);
    } else if (tzs.count() > 0) {
        //TODO: support multiple timezones
        QString tz = tzs.at(0);
        if (tz != m_timezone) {
            dataEngine("time")->disconnectSource(m_timezone, this);
            m_timezone = tz;
        }
    } else if (m_timezone != "Local") {
        dataEngine("time")->disconnectSource(m_timezone, this);
        m_timezone = "Local";
        cg.writeEntry("timezone", m_timezone);
    }

    connectToEngine();
    //TODO: Why we don't call updateConstraints?
    constraintsEvent(Plasma::AllConstraints);
    cg.config()->sync();
}

void BinaryClock::paintInterface(QPainter *p, const QStyleOptionGraphicsItem *option,
                                 const QRect &contentsRect)
{
    Q_UNUSED(option);

    QSizeF m_size = contentsRect.size();
    int appletHeight = (int) m_size.height();
    int appletWidth = (int) m_size.width();
    int dots = m_showSeconds ? 6 : 4;

    int rectSize = (appletHeight - 3) / 4;
    int yPos = (appletHeight % rectSize) / 2;
    int xPos = (appletWidth - (rectSize * dots) - 5) / 2;

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
