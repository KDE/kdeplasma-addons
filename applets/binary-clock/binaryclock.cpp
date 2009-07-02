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

#include "binaryclock.h"

#include <QPainter>

#include <KConfigDialog>

#include <Plasma/DataEngine>
#include <Plasma/Theme>

BinaryClock::BinaryClock(QObject *parent, const QVariantList &args)
    : ClockApplet(parent, args),
      m_showSeconds(true),
      m_showOffLeds(true),
      m_showGrid(true),
      m_time(0, 0)
{
    KGlobal::locale()->insertCatalog("libplasmaclock");
    KGlobal::locale()->insertCatalog("timezones4");

    setHasConfigurationInterface(true);
    resize(getWidthFromHeight(128), 128);
}

void BinaryClock::init()
{
    ClockApplet::init();
    KConfigGroup cg = config();
    m_showSeconds = cg.readEntry("showSeconds", m_showSeconds);
    m_showGrid = cg.readEntry("showGrid", m_showGrid);
    m_showOffLeds = cg.readEntry("showOffLeds", m_showOffLeds);

    m_customOnLedsColor = cg.readEntry("customOnLedsColor", false);
    m_customOffLedsColor = cg.readEntry("customOffLedsColor", false);
    m_customGridColor = cg.readEntry("customGridColor", false);

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
            setMaximumHeight(getHeightFromWidth((int) contentsRect().width()) + borderHeight);

        } else if (formFactor() == Plasma::Horizontal) {
            setMaximumWidth(getWidthFromHeight((int) contentsRect().height()) + borderWidth);

        } else {
            resize(getWidthFromHeight((int) contentsRect().height()) + borderWidth, contentsRect().height() + borderHeight);
            emit appletTransformedItself();
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

    if (Plasma::ToolTipManager::self()->isVisible(this)) {
        updateTipContent();
    }

    m_lastTimeSeen = m_time;

    update();
}

void BinaryClock::createClockConfigurationInterface(KConfigDialog *parent)
{
    QWidget *widget = new QWidget();
    ui.setupUi(widget);
    parent->addPage(widget, i18n("General"), icon());

    ui.showSecondHandCheckBox->setChecked(m_showSeconds);
    ui.showGridCheckBox->setChecked(m_showGrid);
    ui.showOffLedsCheckBox->setChecked(m_showOffLeds);

    QButtonGroup *onLedsGroup = new QButtonGroup(widget);
    onLedsGroup->addButton(ui.onLedsDefaultColorRadioButton);
    onLedsGroup->addButton(ui.onLedsCustomColorRadioButton);

    QButtonGroup *offLedsGroup = new QButtonGroup(widget);
    offLedsGroup->addButton(ui.offLedsDefaultColorRadioButton);
    offLedsGroup->addButton(ui.offLedsCustomColorRadioButton);

    ui.onLedsDefaultColorRadioButton->setChecked(!m_customOnLedsColor);
    ui.offLedsDefaultColorRadioButton->setChecked(!m_customOffLedsColor);
    ui.gridDefaultColorRadioButton->setChecked(!m_customGridColor);

    ui.onLedsCustomColorRadioButton->setChecked(m_customOnLedsColor);
    ui.offLedsCustomColorRadioButton->setChecked(m_customOffLedsColor);
    ui.gridCustomColorRadioButton->setChecked(m_customGridColor);

    KConfigGroup cg = config();
    ui.onLedsCustomColorButton->setColor(cg.readEntry("onLedsColor", m_onLedsColor));
    ui.offLedsCustomColorButton->setColor(cg.readEntry("offLedsColor", m_offLedsColor));
    ui.gridCustomColorButton->setColor(cg.readEntry("gridColor", m_gridColor));
}

void BinaryClock::clockConfigAccepted()
{
    KConfigGroup cg = config();
    m_showSeconds = ui.showSecondHandCheckBox->isChecked();
    m_showGrid = ui.showGridCheckBox->isChecked();
    m_showOffLeds = ui.showOffLedsCheckBox->isChecked();

    m_customOnLedsColor = ui.onLedsCustomColorRadioButton->isChecked();
    m_customOffLedsColor = ui.offLedsCustomColorRadioButton->isChecked();
    m_customGridColor = ui.gridCustomColorRadioButton->isChecked();

    if (m_customOnLedsColor){
         m_onLedsColor = ui.onLedsCustomColorButton->color();
    }

    if (m_customOffLedsColor){
         m_offLedsColor = ui.offLedsCustomColorButton->color();
    }

    if (m_customGridColor){
         m_gridColor = ui.gridCustomColorButton->color();
    }

    cg.writeEntry("showSeconds", m_showSeconds);
    cg.writeEntry("showGrid", m_showGrid);
    cg.writeEntry("showOffLeds", m_showOffLeds);

    cg.writeEntry("customOnLedsColor", m_customOnLedsColor);
    cg.writeEntry("customOffLedsColor", m_customOffLedsColor);
    cg.writeEntry("customGridColor", m_customGridColor);

    cg.writeEntry("onLedsColor", ui.onLedsCustomColorButton->color());
    cg.writeEntry("offLedsColor", ui.offLedsCustomColorButton->color());
    cg.writeEntry("gridColor", ui.gridCustomColorButton->color());

    dataEngine("time")->disconnectSource(currentTimezone(), this);
    connectToEngine();

    updateColors();

    constraintsEvent(Plasma::AllConstraints);
    update();
    emit configNeedsSaving();
}

void BinaryClock::changeEngineTimezone(const QString &oldTimezone, const QString &newTimezone)
{
    dataEngine("time")->disconnectSource(oldTimezone, this);

    Plasma::DataEngine* timeEngine = dataEngine("time");
    if (m_showSeconds) {
        timeEngine->connectSource(newTimezone, this, 500);
    } else {
        timeEngine->connectSource(newTimezone, this, 6000, Plasma::AlignToMinute);
    }
}

void BinaryClock::updateColors()
{
    KConfigGroup cg = config();

    m_onLedsColor = QColor(Plasma::Theme::defaultTheme()->color(Plasma::Theme::TextColor));

    if (m_customOnLedsColor){
        m_onLedsColor = cg.readEntry("onLedsColor", m_onLedsColor);
    }

    m_offLedsColor = QColor(m_onLedsColor);
    m_offLedsColor.setAlpha(40);

    if (m_customOffLedsColor){
        m_offLedsColor = cg.readEntry("offLedsColor", m_offLedsColor);
    }

    m_gridColor = QColor(m_onLedsColor);
    m_gridColor.setAlpha(60);

    if (m_customGridColor){
        m_gridColor = cg.readEntry("gridColor", m_gridColor);
    }

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

    int rectSize = qMax(1, qMin((appletHeight - 3) / 4, (appletWidth - 3) / dots));
    int yPos = ((appletHeight - 4 * rectSize) / 2) + contentsRect.topLeft().y();
    int xPos = ((appletWidth - (rectSize * dots) - 5) / 2) + contentsRect.topLeft().x();

    char timeDigits[6] = {m_time.hour() / 10, m_time.hour() % 10,
                          m_time.minute() / 10, m_time.minute() % 10,
                          m_time.second() / 10, m_time.second() % 10};

    for (int i = 0; i < dots; i++) {
        for (int j = 0; j < 4; j++) {
            if (timeDigits[i] & (1 << (3 - j))) {
                p->fillRect(xPos + (i * (rectSize + 1)), yPos + (j * (rectSize + 1)), rectSize, rectSize, m_onLedsColor);
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
