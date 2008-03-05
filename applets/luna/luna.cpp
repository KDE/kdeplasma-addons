/***************************************************************************
 *   Copyright 1998,2000  Stephan Kulow <coolo@kde.org>                    *
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

#include <assert.h>

#include <QPainter>

#include <Plasma/Svg>
#include <Plasma/Theme>
#include <KDialog>
#include <KDebug>
#include <KDialog>

#include "ui_lunaConfig.h"
#include "phases.cpp"

#include "luna.h"

Luna::Luna(QObject *parent, const QVariantList &args)
    : Plasma::Applet(parent, args),
    m_dialog(0)
{
    setHasConfigurationInterface(true);
    setRemainSquare(true);
    resize(QSize(128, 128));

    counter = -1;
}

void Luna::init()
{
    m_theme = new Plasma::Svg("widgets/luna", this);
    m_theme->setContentType(Plasma::Svg::ImageSet);

    if (!m_theme->isValid()) {
        setFailedToLaunch(true, i18n("The luna SVG file was not found"));
        return;
    }

    KConfigGroup cg = config();

    northHemisphere = cg.readEntry("northHemisphere", true);

    connectToEngine();
}

Luna::~Luna()
{

}

void Luna::connectToEngine()
{
    Plasma::DataEngine* timeEngine = dataEngine("time");
    timeEngine->connectSource("UTC", this, 360000, Plasma::AlignToHour);
}

void Luna::dataUpdated(const QString& source, const Plasma::DataEngine::Data &data)
{
    Q_UNUSED(source)

    QDateTime dt(data["Date"].toDate(), data["Time"].toTime());
    calcStatus(dt.toTime_t());
}

void Luna::showConfigurationInterface()
{
    if (m_dialog == 0) {
        m_dialog = new KDialog;
        m_dialog->setCaption( i18nc("@title:window","Configure Luna") );
        ui.setupUi(m_dialog->mainWidget());
        m_dialog->mainWidget()->layout()->setMargin(0);
        m_dialog->setButtons( KDialog::Ok | KDialog::Cancel | KDialog::Apply );

        connect( m_dialog, SIGNAL(applyClicked()), this, SLOT(configAccepted()) );
        connect( m_dialog, SIGNAL(okClicked()), this, SLOT(configAccepted()) );
    }

    ui.northenRadio->setChecked(northHemisphere);
    ui.southernRadio->setChecked(!northHemisphere);

    m_dialog->show();
}

void Luna::configAccepted()
{
    KConfigGroup cg = config();

    northHemisphere = ui.northenRadio->isChecked();

    cg.writeEntry("northHemisphere", northHemisphere);

    update();

    constraintsUpdated(Plasma::AllConstraints);

    cg.config()->sync();
}

void Luna::paintInterface(QPainter *p, const QStyleOptionGraphicsItem *option,
const QRect &contentsRect)
{
    Q_UNUSED(option)

    if (northHemisphere){
        m_theme->paint(p, contentsRect, QString::number(counter));

    }else{
        p->save();
        p->rotate(180);
        p->translate(-contentsRect.width(), -contentsRect.height());
        m_theme->paint(p, contentsRect, QString::number(counter));
        p->restore();
    }
}

void Luna::calcStatus(time_t time)
{
    Plasma::ToolTipData::ToolTipData toolTipData;

    uint lun = 0;
    time_t last_new = 0;
    time_t next_new = 0;

    do {
        double JDE = moonphasebylunation(lun, 0);
        last_new = next_new;
        next_new = JDtoDate(JDE, 0);
        lun++;
    } while (next_new < time);

    lun -= 2;

    QDateTime ln;
    ln.setTime_t( last_new );
    kDebug() << "last new " << KGlobal::locale()->formatDateTime( ln );

    time_t first_quarter = JDtoDate( moonphasebylunation( lun, 1 ), 0 );
    QDateTime fq;
    fq.setTime_t( first_quarter );
    kDebug() << "first quarter " << KGlobal::locale()->formatDateTime( fq );

    time_t full_moon = JDtoDate( moonphasebylunation( lun, 2 ), 0 );
    QDateTime fm;
    fm.setTime_t( full_moon );
    kDebug() << "full moon " << KGlobal::locale()->formatDateTime( fm );

    time_t third_quarter = JDtoDate( moonphasebylunation( lun, 3 ), 0 );
    QDateTime tq;
    tq.setTime_t( third_quarter );
    kDebug() << "third quarter " << KGlobal::locale()->formatDateTime( tq );

    QDateTime nn;
    nn.setTime_t( next_new );
    kDebug() << "next new " << KGlobal::locale()->formatDateTime( nn );

    QDateTime now;
    now.setTime_t( time );
    kDebug() << "now " << KGlobal::locale()->formatDateTime( now );

    counter = ln.daysTo( now );
    kDebug() << "counter " << counter << " " << fm.daysTo( now );

    if ( fm.daysTo( now ) == 0 ) {
        counter = 14;
        toolTipData.mainText = i18n( "Full Moon" );
        return;
    } else if ( counter <= 15 && counter >= 13 ) {
        counter = 14 + fm.daysTo( now );
        kDebug() << "around full moon " << counter;
    }

    int diff = fq.daysTo( now );
    if ( diff  == 0 )
        counter = 7;
    else if ( counter <= 8 && counter >= 6 ) {
        counter = 7 + diff;
         kDebug() << "around first quarter " << counter;
    }

    diff = ln.daysTo( now );
    if ( diff == 0 )
        counter = 0;
    else if ( counter <= 1 || counter >= 28 )
    {
        counter = ( 29 + diff ) % 29;
        diff = -nn.daysTo( now );
        if ( diff == 0 )
            counter = 0;
        else if ( diff < 3 )
            counter = 29 - diff;
        kDebug() << "around new " << counter << " " << diff;
    }

    if ( tq.daysTo( now ) == 0 )
        counter = 21;
    else if ( counter <= 22 && counter >= 20 )
    {
        counter = 21 + tq.daysTo( now );
        kDebug() << "around third quarter " << counter;
    }

    kDebug() << "counter " << counter;

    assert (counter >= 0 && counter < 29);

    switch (counter) {
    case 0:
        toolTipData.mainText = i18n("New Moon");
        return;
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
        toolTipData.mainText = i18np("Waxing Crescent (New Moon was yesterday)", "Waxing Crescent (%1 days since New Moon)", counter );
        break;
    case 7:
        toolTipData.mainText = i18n("First Quarter");
        break;
    case 8:
    case 9:
    case 10:
    case 11:
    case 12:
    case 13:
        toolTipData.mainText = i18np( "Waxing Gibbous (Tomorrow is Full Moon)", "Waxing Gibbous (%1 days to Full Moon)", -fm.daysTo( now ) );
        break;
    case 14:
        assert( false );
        break;
    case 15:
    case 16:
    case 17:
    case 18:
    case 19:
    case 20:
        toolTipData.mainText = i18np("Waning Gibbous (Yesterday was Full Moon)", "Waning Gibbous (%1 days since Full Moon)", fm.daysTo( now ) );
        break;
    case 21:
        toolTipData.mainText = i18n("Last Quarter");
        break;
    case 22:
    case 23:
    case 24:
    case 25:
    case 26:
    case 27:
    case 28:
        kDebug() << "nn.days " << ln.daysTo( now ) << " " << nn.daysTo( now );
        toolTipData.mainText = i18np("Waning Crescent (Tomorrow is New Moon)", "Waning Crescent (%1 days to New Moon)", -nn.daysTo( now ) );
        break;
    default:
        kFatal() << "coolo can't count\n";
    }

    setToolTip(toolTipData);
}

#include "luna.moc"
