
/**********************************************************************/
/*   TimeMon (c)  1994  Helmut Maierhofer                             */
/*   KDE-ified M. Maierhofer 1998                                     */
/*   maintained by Dirk A. Mueller <mueller@kde.org>                  */
/**********************************************************************/

/*
 * timemon.h
 *
 * Definitions for the timemon widget.
 */


#include <qpainter.h>
#include <qtimer.h>
#include <qtooltip.h>
#include <QPaintEvent>
#include <QPixmap>
#include <QMouseEvent>
#include <QGraphicsView>

#include <kconfig.h>
#include <kglobal.h>
#include <klocale.h>
#include <khelpmenu.h>
#include <kmenu.h>
#include <kprocess.h>
#include <kmessagebox.h>
#include <kdebug.h>
#include "timemon.h"
#include "sample.h"

#include <stdio.h>

#include <plasma/theme.h>
#include <plasma/tooltipmanager.h>
#include <kconfigdialog.h>


void KTimeMon::init()
{
    m_icon = new KIcon("ktimemon"); // TODO: make member (for caching)
    //    m_resultsLabel = new DraggableLabel(this);
    //    m_resultsLabel->setVisible(false);
    //    connect(m_resultsLabel, SIGNAL(linkActivated(QString)), this, SLOT(openLink(QString)));
    connect(Plasma::Theme::defaultTheme(), SIGNAL(themeChanged()), SLOT(updateTheme()));
    updateTip();
    setBackgroundHints(NoBackground);
}


// Update colour settings with the new ones from the config dialog.
void KTimeMon::updateConfig()
{
    kernelColour = uiConfig.CpuKernel_kcolorbutton->color();
    userColour = uiConfig.CpuUser_kcolorbutton->color();
    iowaitColour = uiConfig.CpuIowait_kcolorbutton->color();
    niceColour = uiConfig.CpuNice_kcolorbutton->color();
    cachedColour = uiConfig.MemCached_kcolorbutton->color();
    usedColour = uiConfig.MemUsed_kcolorbutton->color();
    buffersColour = uiConfig.MemBuffers_kcolorbutton->color();
    mkernelColour = uiConfig.MemKernel_kcolorbutton->color();
    swapColour = uiConfig.SwapSwap_kcolorbutton->color();
    bgColour = uiConfig.SwapBackground_kcolorbutton->color();
    vertical = uiConfig.HorizontalBarsCheckBox->isChecked();
}

// -----------------------------------------------------------------------------
// some KPanelApplet API functions

int KTimeMon::widthForHeight(int height) const
{
    int s = (int) (vertical ? 2/3.*height : height);
    return s>=18? s : 18;
}


int KTimeMon::heightForWidth(int width) const
{
    int s = (int) (vertical ? width : 2/3.*width);
    return s>=18? s : 18;
}


void KTimeMon::createConfigurationInterface(KConfigDialog *parent)
{

    QWidget *widget = new QWidget(parent);
    uiConfig.setupUi(widget);
    parent->addPage(widget, i18n("General"), "ktimemon");
    connect((QObject *)parent, SIGNAL(applyClicked()), this, SLOT(configAccepted()));
    connect((QObject *)parent, SIGNAL(okClicked()), this, SLOT(configAccepted()));
    connect(uiConfig.autoScaleBox, SIGNAL(toggled(bool)), this, SLOT(toggleScale(bool)));

    uiConfig.intervalEdit->setValue(interval);
    uiConfig.IntervalHorizontalSlider->setValue(interval);
    uiConfig.CpuKernel_kcolorbutton->setColor(kernelColour);
    uiConfig.CpuUser_kcolorbutton->setColor(userColour);
    uiConfig.CpuNice_kcolorbutton->setColor(niceColour);
    uiConfig.CpuIowait_kcolorbutton->setColor(iowaitColour);
    uiConfig.MemBuffers_kcolorbutton->setColor(buffersColour);
    uiConfig.MemKernel_kcolorbutton->setColor(mkernelColour);
    uiConfig.MemUsed_kcolorbutton->setColor(usedColour);
    uiConfig.MemCached_kcolorbutton->setColor(cachedColour);
    uiConfig.SwapSwap_kcolorbutton->setColor(swapColour);
    uiConfig.SwapBackground_kcolorbutton->setColor(bgColour);
    uiConfig.pageScaleEdit->setValue(pageScale);
    uiConfig.swapScaleEdit->setValue(swapScale);
    uiConfig.ctxScaleEdit->setValue(ctxScale);
    uiConfig.autoScaleBox->setChecked(autoScale);
    uiConfig.HorizontalBarsCheckBox->setChecked(!vertical);
}

// Apply the settings from the configuration dialog and save them.
void KTimeMon::configAccepted()
{
    stop();
    interval = uiConfig.intervalEdit->value();
    cont();

    updateConfig();
    sample->setScaling(uiConfig.autoScaleBox->isChecked(),uiConfig.pageScaleEdit->value(),uiConfig.swapScaleEdit->value(), uiConfig.ctxScaleEdit->value());

    vertical = !uiConfig.HorizontalBarsCheckBox->isChecked();

    update();

    mouseAction[0] = (MouseAction)uiConfig.LeftMouseActionComboBox->currentIndex();
    mouseAction[1] = (MouseAction)uiConfig.MiddleMouseActionComboBox->currentIndex();
    mouseAction[2] = (MouseAction)uiConfig.RightMouseActionComboBox->currentIndex();
    writeConfiguration();
}


void KTimeMon::toggleScale(bool state)
{
    uiConfig.swapScaleEdit->setEnabled(!state);
    uiConfig.pageScaleEdit->setEnabled(!state);
    uiConfig.ctxScaleEdit->setEnabled(!state);
}

// -----------------------------------------------------------------------------
// Repaint the object; get the current sample and paint the bar graphs
// correspondingly. Use a pixmap to minimise flicker.

void KTimeMon::paintInterface(QPainter *p, const QStyleOptionGraphicsItem *, const QRect &contentsRect)
{
  //KMessageBox::information(0, "paintevent!");
    int w, h, x, y, b, r;

    int edgespace=10; // space from edge

    w = vertical ? contentsRect.width()-edgespace : contentsRect.height()-edgespace;
    h = vertical ? contentsRect.height()-edgespace : contentsRect.width()-edgespace;

    r = w;  // remaining height

    KSample::Sample s;

    if (sample != 0)
        s = sample->getSample(h);
    else
        s.fill(h);

    b = r / 3;            // bar width
    r -= b;

    if (bgColour != Plasma::Theme::defaultTheme()->color(Plasma::Theme::BackgroundColor))
    {
        paintRect(x, 0, b, h, bgColour, p);
    }

    x =edgespace;
    y = h+edgespace;
    
    y -= s.kernel;
    paintRect(x, y, b, s.kernel, kernelColour, p);
    y -= s.iowait;
    paintRect(x, y, b, s.iowait, iowaitColour, p);
    y -= s.user;
    paintRect(x, y, b, s.user, userColour, p);
    y -= s.nice;
    paintRect(x, y, b, s.nice, niceColour, p);

    x += b;
    b = r / 2;
    r -= b;

    if (bgColour != Plasma::Theme::defaultTheme()->color(Plasma::Theme::BackgroundColor))
    {
        paintRect(x, 0, b, h, bgColour, p);
    }

    y = h+edgespace;
    y -= s.mkernel;
    paintRect(x, y, b, s.mkernel, mkernelColour, p);
    y -= s.used;
    paintRect(x, y, b, s.used, usedColour, p);
    y -= s.buffers;
    paintRect(x, y, b, s.buffers, buffersColour, p);
    y -= s.cached;
    paintRect(x, y, b, s.cached, cachedColour, p);

    x += b;
    b = r;

    if (bgColour != Plasma::Theme::defaultTheme()->color(Plasma::Theme::BackgroundColor))
    {
        paintRect(x, 0, b, h, bgColour, p);
    }

    y = h+edgespace;
    y -= s.sused;
    paintRect(x, y, b, s.sused, swapColour, p);

    updateTip();
}

// -----------------------------------------------------------------------------
// Draw part of a bar, depending on the bar orientation.

void KTimeMon::paintRect(int x, int y, int w, int h, QColor c, QPainter *p)
{
    if (vertical)
        p->fillRect(x, y, w, h, c);
    else
        p->fillRect(geometry().width() - y - h, x, h, w, c);
}

// Show a tool-tip with some status information.
void KTimeMon::updateTip()
{
    if (sample == 0) return;    // no associated sample...

    KSample::Sample s = sample->getSample(100); // scale to 100(%)
    int idle = 100 - s.kernel - s.user - s.nice;
    if ( idle < 0 )
        idle = 0;
    QString str = i18n("cpu: %1% idle\nmem: %2 MB %3% free\nswap: %4 MB %5% free")
                .arg(idle)
                .arg(KGlobal::locale()->formatNumber(s.used/100.*s.mtotal, 0))
                .arg(100-s.used)
                .arg(KGlobal::locale()->formatNumber(s.stotal, 0))
                .arg(100-s.sused);

    Plasma::ToolTipContent toolTipData(i18n("Info"), str);
    Plasma::ToolTipManager::self()->setContent(this, toolTipData);
}

// -- KTimeMon definition ------------------------------------------------

// Initialise the member variables, read the configuration data base,
// set up the widget, and start the timer.
KTimeMon::KTimeMon(QObject *parent, const QVariantList &args) : Plasma::Applet(parent, args),
    bgProcess(0),
    kernelColour( Qt::red), userColour( Qt::blue),
    niceColour( Qt::yellow), iowaitColour( Qt::green),
    usedColour( Qt::blue), buffersColour( Qt::yellow),
    cachedColour( Qt::green), mkernelColour( Qt::red),
    swapColour(Qt::cyan), bgColour(Plasma::Theme::defaultTheme()->color(Plasma::Theme::BackgroundColor))
{
    mouseAction[0] = NOTHING;
    mouseAction[1] = NOTHING;
    mouseAction[2] = MENU;


    KConfigGroup conf = KGlobal::config()->group("Parameters");
    interval = QVariant(conf.readEntry("Interval", 500)).toInt();
    autoScale = QVariant(conf.readEntry("AutoScale", true)).toBool();

    pageScale = QVariant(conf.readEntry("PageScale", 10)).toInt();
    swapScale = QVariant(conf.readEntry("SwapScale", 5)).toInt();
    ctxScale = QVariant(conf.readEntry("ContextScale", 300)).toInt();
    for (int i = 0; i < MAX_MOUSE_ACTIONS; i++) {
        QString n;
        n.setNum(i);

        mouseAction[i] = (MouseAction) QVariant(conf.readEntry(QString("MouseAction")+n, (int)mouseAction[i])).toInt();
        mouseActionCommand[i] = (QString)QVariant(conf.readEntry(QString("MouseActionCommand")+n)).toString();
    }

    conf = KGlobal::config()->group("Interface");
    kernelColour = QVariant(conf.readEntry("KernelColour", kernelColour)).value<QColor>();
    userColour = QVariant(conf.readEntry("UserColour", userColour)).value<QColor>();
    niceColour = QVariant(conf.readEntry("NiceColour", niceColour)).value<QColor>();
    iowaitColour = QVariant(conf.readEntry("IOWaitColour", iowaitColour)).value<QColor>();
    cachedColour = QVariant(conf.readEntry("CachedColour", cachedColour)).value<QColor>();
    usedColour = QVariant(conf.readEntry("UsedColour", usedColour)).value<QColor>();
    buffersColour = QVariant(conf.readEntry("BuffersColour", buffersColour)).value<QColor>();
    swapColour = QVariant(conf.readEntry("SwapColour",swapColour)).value<QColor>();
    mkernelColour = QVariant(conf.readEntry("MKernelColour", mkernelColour)).value<QColor>();
    bgColour = QVariant(conf.readEntry("BgColour", bgColour)).value<QColor>();

    vertical = QVariant(conf.readEntry("Vertical", true)).toBool();

    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(timeout()));
    timer->start(interval);

    sample = new KSample(this, autoScale, pageScale, swapScale, ctxScale);
    //vertical = !vertical;               // and similar for orientation
    orientation();

    KGlobal::locale()->insertCatalog("libplasmaktimemon");

    setHasConfigurationInterface(true);
    resize(64, 64);

}

// -----------------------------------------------------------------------------

// delete the member variables
KTimeMon::~KTimeMon()
{
    delete sample;
    delete bgProcess;
    //FIXME
    //KGlobal::locale()->removeCatalogue("ktimemon");

    if (hasFailedToLaunch()) {
        // Do some cleanup here
    } else {
	// TODO
    }

}

void KTimeMon::stop()
{
    timer->stop();
}

void KTimeMon::cont()
{
    timer->start(interval);
}

// Dump the current configuration entries to the data base.
void KTimeMon::writeConfiguration()
{
    KConfigGroup conf = KGlobal::config()->group("Interface");
    conf.writeEntry("KernelColour", kernelColour);
    conf.writeEntry("UserColour", userColour);
    conf.writeEntry("NiceColour", niceColour);
    conf.writeEntry("IOWaitColour", iowaitColour);
    conf.writeEntry("CachedColour", cachedColour);
    conf.writeEntry("UsedColour", usedColour);
    conf.writeEntry("BuffersColour", buffersColour);
    conf.writeEntry("MKernelColour", mkernelColour);
    conf.writeEntry("SwapColour", swapColour);
    conf.writeEntry("BgColour", bgColour);
    conf.writeEntry("Mode", true);
    conf.writeEntry("Vertical", vertical);

    conf = KGlobal::config()->group("Parameters");
    conf.writeEntry("Interval", interval);
    conf.writeEntry("AutoScale", autoScale);
    conf.writeEntry("PageScale", pageScale);
    conf.writeEntry("SwapScale", swapScale);
    conf.writeEntry("ContextScale", ctxScale);
    conf.writeEntry("WidgetSize", size());
    for (int i = 0; i < MAX_MOUSE_ACTIONS; i++) {
        QString n;
        n.setNum(i);

        conf.writeEntry(QString("MouseAction")+n, (unsigned)mouseAction[i]);
        conf.writePathEntry(QString("MouseActionCommand")+n, mouseActionCommand[i]);
    }
    conf.sync();
}

// Make the KSample object update its internal sample and repaint the
// object.
void KTimeMon::timeout()
{
    sample->updateSample();
    update();
}

// This is called when the session management strikes, and also when the
// main program exits with a code of 0 (i.e. there was no error).
void KTimeMon::save()
{
    writeConfiguration();
}

// -----------------------------------------------------------------------------
// Update the configuration dialog with the current values and show it.

// -----------------------------------------------------------------------------
// Change the orientation of the status bars

void KTimeMon::orientation()
{
    vertical = !vertical;

    KConfigGroup conf = KGlobal::config()->group("Interface");
    conf.writeEntry("Vertical", vertical);

    // FIXME
    //menu->setItemChecked(4, !vertical);
    
    // FIXME
    //emit updateLayout();
}

// Pop up the menu when the appropriate button has been pressed.
void KTimeMon::mousePressEvent(QGraphicsSceneMouseEvent *event)
{


    if (event == 0) return;

    int index = -1;
    if (event->button() == Qt::LeftButton)
      index = 0;
    else if (event->button() == Qt::MidButton)
      index = 1;
    else if (event->button() == Qt::RightButton)
      index = 2;
    
    if (index == -1) return;

    switch (mouseAction[index]) {
    case NOTHING:
        break;
    case MENU:
	break;
    case SWITCH:
      menu = new KMenu(0);
      menu->addTitle( SmallIcon( "ktimemon" ), i18n( "System Monitor" ) ) ;
      if (vertical)
	action = menu->addAction(i18n("Horizontal Bars"));
      else
	action = menu->addAction(i18n("Vertical Bars"));
      connect(action, SIGNAL(triggered(bool)), this, SLOT(orientation()));
      action = menu->addAction(SmallIcon( "ktimemon" ), i18n( "About..." ));
      connect(action, SIGNAL(triggered(bool)), this, SLOT(about()));
	menu->setCheckable(true);
       menu->popup(view()->mapFromScene(view()->mapToGlobal(event->pos().toPoint())), 0 );
    break;
    case COMMAND:
        runCommand(index);
        break;
    }
}

// Start the given command
void KTimeMon::runCommand(int index)
{
    // just in case it still hangs around
    if (bgProcess != NULL)
      delete bgProcess;

    bgProcess = new KProcess;
    *bgProcess << mouseActionCommand[index];
    connect(bgProcess, SIGNAL( readyReadStandardError()), this, SLOT(commandStderr()));
    bgProcess->start();
}

// -----------------------------------------------------------------------------
// Check if there is any diagnostic output (command not found or such)

void KTimeMon::commandStderr()
{
    QString buffer = QString(bgProcess->readAllStandardError());

    QString msgbuf  = i18n("Got diagnostic output from child command:\n\n");
    msgbuf += buffer;

    KMessageBox::information(0, msgbuf);
}

// -----------------------------------------------------------------------------

void KTimeMon::about()
{
      QString aboutmsg = i18n("KTimeMon for KDE\n"
			    "Ported to KDE4 bys Christoph Thielecke <christoph.thielecke@gmx.de>\n"
			    "Maintained by Dirk A. Mueller <dmuell@gmx.net>\n"
			    "Written by M. Maierhofer (m.maierhofer@tees.ac.uk)\n"
			    "Based on timemon by H. Maierhofer");

      KMessageBox::information(0,aboutmsg);
}


K_EXPORT_PLASMA_APPLET(ktimemon, KTimeMon)
