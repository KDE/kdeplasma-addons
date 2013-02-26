/****************************************************************************
* System Monitor: Plasmoid and data engines to monitor CPU/Memory/Swap Usage.
* Copyright (C) 2008  Matthew Dawson <matthewjd@gmail.com>
* Copyright (C) 2008-2009 Dominik Haumann <dhaumann kde org>
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either version 2
* of the License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*
*****************************************************************************/

#include "systemloadviewer.h"
#include "systemloadviewer.moc"

#include <Plasma/ToolTipManager>
#include <Plasma/ToolTipContent>

#include <KConfigDialog>
#include <KColorUtils>
#include <KDebug>

#include <QtGui/QPainter>
#include <QtGui/QGraphicsSceneMouseEvent>
#include <QtDBus/QDBusInterface>

SystemLoadViewer::SystemLoadViewer(QObject *parent, const QVariantList &args)
    : Plasma::Applet(parent, args)
    , m_numCPUs(0)
    , m_ramfree(0)
    , m_ramused(0)
    , m_ramapps(0)
    , m_rambuffers(0)
    , m_ramcached(0)
    , m_ramtotal(1)
    , m_swapfree(0)
    , m_swapused(0)
    , m_swaptotal(1)
    , m_barOrientation(Qt::Vertical)
    , m_showMultiCPU(false)
    , m_swapAvailable(true)
    , m_updateInterval(500)
    , sys_mon(0)
{
    m_cpuInfo.resize(1);
    setHasConfigurationInterface(true);

    setAspectRatioMode(Plasma::IgnoreAspectRatio);
    resize(100, 100); // sane size for Planar and MediaCenter

    m_freeResourceColor = QColor(128, 128, 128, 128);
}

SystemLoadViewer::~SystemLoadViewer()
{
}

void SystemLoadViewer::init()
{
    configChanged();

    sys_mon = dataEngine("systemmonitor");
    connect(sys_mon, SIGNAL(sourceAdded(QString)), this, SLOT(sourcesAdded(QString)));
    reconnectSources();

    Plasma::ToolTipManager::self()->registerWidget(this);
}

void SystemLoadViewer::reconnectSources()
{
    reconnectCPUSources();

    sys_mon->connectSource("mem/physical/application", this, m_updateInterval);
    sys_mon->connectSource("mem/physical/used", this, m_updateInterval);
    sys_mon->connectSource("mem/physical/free", this, m_updateInterval);
    sys_mon->connectSource("mem/physical/buf", this, m_updateInterval);
    sys_mon->connectSource("mem/physical/cached", this, m_updateInterval);

    sys_mon->connectSource("mem/swap/used", this, m_updateInterval);
    sys_mon->connectSource("mem/swap/free", this, m_updateInterval);
    
    //Watch this cpu source here, as it doesn't need disconnection when the other cpu ones do.
    sys_mon->connectSource("system/cores", this, m_updateInterval);
}

void SystemLoadViewer::reconnectCPUSources()
{
    sys_mon->connectSource("cpu/system/user", this, m_updateInterval);
    sys_mon->connectSource("cpu/system/sys", this, m_updateInterval);
    sys_mon->connectSource("cpu/system/nice", this, m_updateInterval);
    sys_mon->connectSource("cpu/system/wait", this, m_updateInterval);
    sys_mon->connectSource("cpu/system/AverageClock", this, m_updateInterval);

    if (m_numCPUs == 0) {
        //If we have zero, either the sources doesn't exist or theres a problem.
        return ;
    }

    m_cpuInfo.resize(m_numCPUs);

    for (uint i = 0; i < m_numCPUs; ++i) {

        sys_mon->connectSource(QString("cpu/cpu%1/user").arg(i), this, m_updateInterval);
        sys_mon->connectSource(QString("cpu/cpu%1/sys").arg(i), this, m_updateInterval);
        sys_mon->connectSource(QString("cpu/cpu%1/nice").arg(i), this, m_updateInterval);
        sys_mon->connectSource(QString("cpu/cpu%1/wait").arg(i), this, m_updateInterval);
        sys_mon->connectSource(QString("cpu/cpu%1/clock").arg(i), this, m_updateInterval);
        m_cpuInfo[i].clockValid = false;

    }
}

void SystemLoadViewer::disconnectSources()
{
    disconnectCPUSources();

    sys_mon->disconnectSource("mem/physical/application", this);
    sys_mon->disconnectSource("mem/physical/used", this);
    sys_mon->disconnectSource("mem/physical/free", this);
    sys_mon->disconnectSource("mem/physical/buf", this);
    sys_mon->disconnectSource("mem/physical/cached", this);

    sys_mon->disconnectSource("mem/swap/used", this);
    sys_mon->disconnectSource("mem/swap/free", this);
    
    for (uint i = 0; i < m_numCPUs; ++i) {

        sys_mon->connectSource(QString("cpu/cpu%1/user").arg(i), this, m_updateInterval);
        sys_mon->connectSource(QString("cpu/cpu%1/sys").arg(i), this, m_updateInterval);
        sys_mon->connectSource(QString("cpu/cpu%1/nice").arg(i), this, m_updateInterval);
        sys_mon->connectSource(QString("cpu/cpu%1/wait").arg(i), this, m_updateInterval);
        sys_mon->connectSource(QString("cpu/cpu%1/clock").arg(i), this, m_updateInterval);
        m_cpuInfo[i].clockValid = false;

    }
    
    //Loose this cpu source here, as it doesn't need disconnection when the other cpu ones do.
    sys_mon->connectSource("system/cores", this);
}

void SystemLoadViewer::disconnectCPUSources()
{
    sys_mon->disconnectSource("cpu/system/user", this);
    sys_mon->disconnectSource("cpu/system/sys", this);
    sys_mon->disconnectSource("cpu/system/nice", this);
    sys_mon->disconnectSource("cpu/system/wait", this);
    sys_mon->disconnectSource("cpu/system/AverageClock", this);
    m_systemCpuInfo.clockValid = false;

    for (uint i = 0; i < m_numCPUs; ++i) {

        sys_mon->disconnectSource(QString("cpu/cpu%1/user").arg(i), this);
        sys_mon->disconnectSource(QString("cpu/cpu%1/sys").arg(i), this);
        sys_mon->disconnectSource(QString("cpu/cpu%1/nice").arg(i), this);
        sys_mon->disconnectSource(QString("cpu/cpu%1/wait").arg(i), this);
        sys_mon->disconnectSource(QString("cpu/cpu%1/clock").arg(i), this);
        m_cpuInfo[i].clockValid = false;

    }
}

void SystemLoadViewer::sourcesAdded(const QString &source)
{
    //kDebug() <<  source;
    if (source == "system/cores") {

        sys_mon->connectSource(source, this, m_updateInterval);

    } else if (source.startsWith(QLatin1String("cpu/system/"))) {

        if (source.endsWith(QLatin1String("/user")) || source.endsWith(QLatin1String("/sys")) ||
            source.endsWith(QLatin1String("/nice")) || source.endsWith(QLatin1String("/wait")) ||
            source.endsWith(QLatin1String("/AverageClock")))
        {
            sys_mon->connectSource(source, this, m_updateInterval);
        }

    } else if (source.startsWith(QLatin1String("cpu/cpu"))) {

        if (source.endsWith(QLatin1String("/user")) || source.endsWith(QLatin1String("/sys")) ||
            source.endsWith(QLatin1String("/nice")) || source.endsWith(QLatin1String("/wait")) ||
            source.endsWith(QLatin1String("/clock")) )
        {
            sys_mon->connectSource(source, this, m_updateInterval);
        }

    } else if (source.startsWith(QLatin1String("mem/swap/"))) {

        sys_mon->connectSource(source, this, m_updateInterval);

    } else if (source.startsWith(QLatin1String("mem/physical/"))) {

        sys_mon->connectSource(source, this, m_updateInterval);
    }
}

void SystemLoadViewer::constraintsEvent(Plasma::Constraints constraints)
{
    if (constraints.testFlag(Plasma::FormFactorConstraint)) {
        updateSize();

        if (formFactor() == Plasma::Planar || formFactor() == Plasma::MediaCenter) {
            if (backgroundHints() != TranslucentBackground) {
                setBackgroundHints(TranslucentBackground);
            }
        } else if (backgroundHints() != NoBackground) { // else: Horizontal/Vertical
            setBackgroundHints(NoBackground);
        }

    } else if (constraints.testFlag(Plasma::SizeConstraint)) {
        updateSize();
    }
}

void SystemLoadViewer::updateSize()
{
    setMinimumWidth(0);
    setMinimumHeight(0);

    if (formFactor() == Plasma::Horizontal) {
        setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);
        setMinimumWidth(widthForHeight(size().height()));
    } else if (formFactor() == Plasma::Vertical) {
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
        setMinimumHeight(heightForWidth(size().width()));
    } else {
        setPreferredHeight(heightForWidth(size().width()));
    }
}

qreal SystemLoadViewer::widthForHeight(qreal h) const
{
    if (verticalBars()) {
        return h * 0.8 / 3.0 * barCount();
    } else {
        return h / 0.8 * 3.0 / barCount();
    }
}

qreal SystemLoadViewer::heightForWidth(qreal w) const
{
    if (verticalBars()) {
        return w / 0.8 * 3.0 / barCount();
    } else {
        return w * 0.8 / 3.0 * barCount();
    }
}

int SystemLoadViewer::cpuCount() const
{
    return qMax(1, m_showMultiCPU ? (int)m_numCPUs : 1);
}

int SystemLoadViewer::barCount() const
{
    return cpuCount() + 1 + (m_swapAvailable ? 1 : 0); // + 1 = memory
}

QSizeF SystemLoadViewer::sizeHint(Qt::SizeHint which, const QSizeF &constraint) const
{
    QSizeF bounds = constraint.isValid() ? constraint : contentsRect().size();
    QSizeF hint = Applet::sizeHint(which, constraint);
    if (which == Qt::PreferredSize) {
        if (formFactor() == Plasma::Horizontal) {
            hint = QSizeF(widthForHeight(bounds.height()), bounds.height());
        } else if (formFactor() == Plasma::Vertical) {
            hint = QSizeF(bounds.width(), heightForWidth(bounds.width()));
        }
    }

//     kDebug() << "sizeHint:" << hint << " -- which:" << which;
    return hint;
}

void SystemLoadViewer::mousePressEvent(QGraphicsSceneMouseEvent *event){
 
    if(event->button() == Qt::LeftButton){
        m_mousePressLoc = event->screenPos();
    }
  
}

void SystemLoadViewer::mouseReleaseEvent(QGraphicsSceneMouseEvent *event){
 
  if(event->button() == Qt::LeftButton && ((event->screenPos() - m_mousePressLoc).manhattanLength()) < QApplication::startDragDistance()){
      if(hasAuthorization("LaunchApp")){
        QDBusInterface("org.kde.krunner", "/App", "org.kde.krunner.App").call(QDBus::NoBlock, "showTaskManager");
      }
  }
  
}

void SystemLoadViewer::createConfigurationInterface(KConfigDialog *parent)
{

    QWidget *widGeneral = new QWidget();
    uiGeneral.setupUi(widGeneral);

    uiGeneral.chkIsVertical->setChecked(verticalBars());
    uiGeneral.chkShowAllProcessors->setChecked(m_showMultiCPU);
    uiGeneral.spbxUpdateInterval->setValue(m_updateInterval);

    QWidget *widColours = new QWidget();
    uiColours.setupUi(widColours);

    uiColours.kcbCpuUser->setColor(m_cpuUserColour);
    uiColours.kcbCpuNice->setColor(m_cpuNiceColour);
    uiColours.kcbCpuDisk->setColor(m_cpuDiskColour);
    uiColours.kcbCpuSystem->setColor(m_cpuSysColour);
    uiColours.kcbRamBuffers->setColor(m_ramBuffersColour);
    uiColours.kcbRamCached->setColor(m_ramCachedColour);
    uiColours.kcbRamUser->setColor(m_ramUserColour);
    uiColours.kcbSwap->setColor(m_swapUsedColour);
    uiColours.kcbFreeResourceColour->setColor(m_freeResourceColor);
    uiColours.hsldrFreeResourceTransparency->setValue(255 - m_freeResourceColor.alpha());

    uiColours.kcbSwap->setEnabled(m_swapAvailable);
    uiColours.lblSwapUsed->setEnabled(m_swapAvailable);

    parent->setButtons(KDialog::Ok | KDialog::Cancel | KDialog::Apply);
    connect(parent, SIGNAL(applyClicked()), this, SLOT(configUpdated()));
    connect(parent, SIGNAL(okClicked()), this, SLOT(configUpdated()));

    parent->addPage(widGeneral, i18nc("@title:group General options", "General"), icon(), QString(), false);
    parent->addPage(widColours, i18nc("@title:group", "Colors"), icon(), QString(), false);

    connect(uiGeneral.chkIsVertical, SIGNAL(toggled(bool)), parent, SLOT(settingsModified()));
    connect(uiGeneral.chkShowAllProcessors, SIGNAL(toggled(bool)), parent, SLOT(settingsModified()));
    connect(uiGeneral.spbxUpdateInterval, SIGNAL(valueChanged(int)), parent, SLOT(settingsModified()));
    connect(uiColours.kcbFreeResourceColour, SIGNAL(changed(QColor)), parent, SLOT(settingsModified()));
    connect(uiColours.kcbCpuNice, SIGNAL(changed(QColor)), parent, SLOT(settingsModified()));
    connect(uiColours.kcbCpuUser, SIGNAL(changed(QColor)), parent, SLOT(settingsModified()));
    connect(uiColours.kcbCpuDisk, SIGNAL(changed(QColor)), parent, SLOT(settingsModified()));
    connect(uiColours.kcbCpuSystem, SIGNAL(changed(QColor)), parent, SLOT(settingsModified()));
    connect(uiColours.kcbRamBuffers, SIGNAL(changed(QColor)), parent, SLOT(settingsModified()));
    connect(uiColours.kcbRamCached, SIGNAL(changed(QColor)), parent, SLOT(settingsModified()));
    connect(uiColours.kcbRamUser, SIGNAL(changed(QColor)), parent, SLOT(settingsModified()));
    connect(uiColours.kcbSwap, SIGNAL(changed(QColor)), parent, SLOT(settingsModified()));
    connect(uiColours.hsldrFreeResourceTransparency, SIGNAL(valueChanged(int)), parent, SLOT(settingsModified()));
}

void SystemLoadViewer::configUpdated()
{
    KConfigGroup cg = config();

    if (uiGeneral.chkIsVertical->isChecked() != verticalBars()) {
        setVerticalBars(uiGeneral.chkIsVertical->isChecked());
        cg.writeEntry("vertical", verticalBars());
    }
    if (uiGeneral.chkShowAllProcessors->isChecked() != m_showMultiCPU) {
        disconnectCPUSources();
        m_showMultiCPU = uiGeneral.chkShowAllProcessors->isChecked();
        cg.writeEntry("show_multiple_cpus", m_showMultiCPU);
        reconnectCPUSources();
    }
    if (uiGeneral.spbxUpdateInterval->value() != m_updateInterval) {
        m_updateInterval = uiGeneral.spbxUpdateInterval->value();
        cg.writeEntry("update_interval", m_updateInterval);
        disconnectSources();
        reconnectSources();
    }
    if (uiColours.kcbCpuUser->color() != m_cpuUserColour) {
        m_cpuUserColour = uiColours.kcbCpuUser->color();
        cg.writeEntry("colour_cpu_user", m_cpuUserColour.name());
    }
    if (uiColours.kcbCpuNice->color() != m_cpuNiceColour) {
        m_cpuNiceColour = uiColours.kcbCpuNice->color();
        cg.writeEntry("colour_cpu_nice", m_cpuNiceColour.name());
    }
    if (uiColours.kcbCpuDisk->color() != m_cpuDiskColour) {
        m_cpuDiskColour = uiColours.kcbCpuDisk->color();
        cg.writeEntry("colour_cpu_disk", m_cpuDiskColour.name());
    }
    if (uiColours.kcbCpuSystem->color() != m_cpuSysColour) {
        m_cpuSysColour = uiColours.kcbCpuSystem->color();
        cg.writeEntry("colour_cpu_sys", m_cpuSysColour.name());
    }
    if (uiColours.kcbRamCached->color() != m_ramCachedColour) {
        m_ramCachedColour = uiColours.kcbRamCached->color();
        cg.writeEntry("colour_ram_cached", m_ramCachedColour.name());
    }
    if (uiColours.kcbRamBuffers->color() != m_ramBuffersColour) {
        m_ramBuffersColour = uiColours.kcbRamBuffers->color();
        cg.writeEntry("colour_ram_buffers", m_ramBuffersColour.name());
    }
    if (uiColours.kcbRamUser->color() != m_ramUserColour) {
        m_ramUserColour = uiColours.kcbRamUser->color();
        cg.writeEntry("colour_ram_used", m_ramUserColour.name());
    }
    if (uiColours.kcbSwap->color() != m_swapUsedColour) {
        m_swapUsedColour = uiColours.kcbSwap->color();
        cg.writeEntry("colour_swap_used", m_swapUsedColour.name());
    }
    //Use the rgb value here instead of directly comparing the colours so that the alpha value is ignored.  The colour coming from the form will not have the correct alpha value set.
    if (uiColours.kcbFreeResourceColour->color().rgb() != m_freeResourceColor.rgb()) {
        m_freeResourceColor = uiColours.kcbFreeResourceColour->color();
        cg.writeEntry("colour_free_resource", m_freeResourceColor.name());
    }
    if (uiColours.hsldrFreeResourceTransparency->value() != (255 - m_freeResourceColor.alpha())){
        m_freeResourceColor.setAlpha(255 - uiColours.hsldrFreeResourceTransparency->value());
        cg.writeEntry("transparency_free_resource", m_freeResourceColor.alpha());        
    }

    emit configNeedsSaving();
    updateConstraints(Plasma::SizeConstraint); // make sure sizes are updated correctly
}

void SystemLoadViewer::dataUpdated(const QString& source, const Plasma::DataEngine::Data &data)
{
    //kDebug() << source << "=" << data["value"].toString();
    if (source == "system/cores") {
        uint newNumCPUs = data["value"].toUInt();
        if (newNumCPUs != m_numCPUs) {
            disconnectCPUSources();
            m_numCPUs = newNumCPUs; //Make the switch here so that we lose all previous connections, otherwise if
                                    //we lose cores we will retain unwanted source connections.
            reconnectCPUSources();
            updateConstraints(Plasma::SizeConstraint);
        }
    } else if (!m_showMultiCPU && source.startsWith(QLatin1String("cpu/system/"))) {
        if (source.endsWith(QLatin1String("/user"))) {
            m_systemCpuInfo.user = (data["value"].toString().toDouble()) / 100;
        } else if (source.endsWith(QLatin1String("/sys"))) {
            m_systemCpuInfo.sys = (data["value"].toString().toDouble()) / 100;
        } else if (source.endsWith(QLatin1String("/nice"))) {
            m_systemCpuInfo.nice = (data["value"].toString().toDouble()) / 100;
        } else if (source.endsWith(QLatin1String("/wait"))) {
            m_systemCpuInfo.disk = (data["value"].toString().toDouble()) / 100;
        } else if (source.endsWith(QLatin1String("/AverageClock"))) {
            m_systemCpuInfo.clockValid = true;
            m_systemCpuInfo.clock = (data["value"].toString().toDouble());
        }
    } else if (m_showMultiCPU && source.startsWith(QLatin1String("cpu/cpu")) && (m_numCPUs != 0)) {
        int cpu = source.split('/')[1].mid(3).toInt();

        if (cpu >= m_cpuInfo.size()) {
            kDebug() << "CPU" << cpu << "is out of range! There are only"
                     << m_cpuInfo.size() << "CPUs.";
            return;
        }

        if (source.endsWith(QLatin1String("/user"))) {
            m_cpuInfo[cpu].user = (data["value"].toString().toDouble()) / 100;
        } else if (source.endsWith(QLatin1String("/sys"))) {
            m_cpuInfo[cpu].sys = (data["value"].toString().toDouble()) / 100;
        } else if (source.endsWith(QLatin1String("/nice"))) {
            m_cpuInfo[cpu].nice = (data["value"].toString().toDouble()) / 100;
        } else if (source.endsWith(QLatin1String("/wait"))) {
            m_cpuInfo[cpu].disk = (data["value"].toString().toDouble()) / 100;
        } else if (source.endsWith(QLatin1String("/clock"))) {
            m_cpuInfo[cpu].clockValid = true;
            m_cpuInfo[cpu].clock = (data["value"].toString().toDouble());
        }
    } else if (source.startsWith(QLatin1String("mem/swap/"))) {
        if (source.endsWith(QLatin1String("/used"))) {
            m_swapused = data["value"].toDouble() / 100;
        } else if (source.endsWith(QLatin1String("/free"))) {
            m_swapfree = data["value"].toDouble() / 100;
        }

        m_swaptotal = m_swapfree + m_swapused;
        if(qFuzzyCompare(m_swaptotal+1, 1.0)) {
            if(m_swapAvailable) {
                m_swapAvailable = false;
                updateConstraints(Plasma::SizeConstraint);
            }
        }else {
            if(!m_swapAvailable) {
                m_swapAvailable = true;
                updateConstraints(Plasma::SizeConstraint);
            }
        }

        m_swaptotal = qMax(0.1, m_swaptotal); //Make sure we never have a zero value for swap total, avoiding division by 0 errors.
    } else if (source.startsWith(QLatin1String("mem/physical/"))) {
        if (source.endsWith(QLatin1String("/application"))) {
            m_ramapps = data["value"].toDouble();
        } else if (source.endsWith(QLatin1String("/used"))) {
            m_ramused = data["value"].toDouble();
        } else if (source.endsWith(QLatin1String("/cached"))) {
            m_ramcached = data["value"].toDouble();
        } else if (source.endsWith(QLatin1String("/buf"))) {
            m_rambuffers = data["value"].toDouble();
        } else if (source.endsWith(QLatin1String("/free"))) {
            m_ramfree = data["value"].toDouble();
        }

        m_ramtotal = qMax(1.0, m_ramused + m_ramfree);
    }

    // if the tooltip is visible, keep the displayed data up-to-date
    if (Plasma::ToolTipManager::self()->isVisible(this)) {
        toolTipAboutToShow();
    }

    update();
}

static inline void drawSection(QPainter* p, const QColor& color, const QRectF& rect)
{
    if (rect.height() > 0.0) {
        static QLinearGradient grad(0.0, 0.0, 1.0, 0.0);
        grad.setColorAt(0, KColorUtils::lighten(color));
        grad.setColorAt(1, KColorUtils::darken(color));
        QBrush brush(grad);
        p->setBrush(brush);
        p->setPen(QPen(brush, 0));
        p->drawRect(rect);
        p->translate(0, rect.height());
    }
}

void SystemLoadViewer::paintCPUUsage(QPainter *p, const QStyleOptionGraphicsItem *option, const QRect& contentsRect, const CpuInfo &cpu)
{
    Q_UNUSED(option)

    p->save();
    p->scale(contentsRect.width(), 1.0);

    qreal height = contentsRect.height() * (1 - (cpu.nice + cpu.user + cpu.disk + cpu.sys)); //Compute idle from components to avoid race conditions.
    drawSection(p, m_freeResourceColor, QRectF(0, contentsRect.top(), 1.0, height));

    height = contentsRect.height() * cpu.nice;
    drawSection(p, m_cpuNiceColour, QRectF(0, contentsRect.top(), 1.0, height));

    height = contentsRect.height() * cpu.user;
    drawSection(p, m_cpuUserColour, QRectF(0, contentsRect.top(), 1.0, height));

    height = contentsRect.height() * cpu.disk;
    drawSection(p, m_cpuDiskColour, QRectF(0, contentsRect.top(), 1.0, height));

    height = contentsRect.height() * cpu.sys;
    drawSection(p, m_cpuSysColour, QRectF(0, contentsRect.top(), 1.0, height));

    p->restore();
}

void SystemLoadViewer::paintSwapUsage(QPainter *p, const QStyleOptionGraphicsItem *option, const QRect& contentsRect)
{
    Q_UNUSED(option)

    p->save();
    p->scale(contentsRect.width(), 1.0);

    qreal height = contentsRect.height() * m_swapfree / m_swaptotal;
    drawSection(p, m_freeResourceColor, QRectF(0, contentsRect.top(), 1.0, height));

    height = contentsRect.height() * m_swapused / m_swaptotal;
    drawSection(p, m_swapUsedColour, QRectF(0, contentsRect.top(), 1.0, height));

    p->restore();
}

void SystemLoadViewer::paintRAMUsage(QPainter *p, const QStyleOptionGraphicsItem *option, const QRect& contentsRect)
{
    Q_UNUSED(option)

    p->save();
    p->scale(contentsRect.width(), 1.0);

    qreal height = contentsRect.height() * m_ramfree / m_ramtotal;
    drawSection(p, m_freeResourceColor, QRectF(0, contentsRect.top(), 1.0, height));

    height = contentsRect.height() * m_ramcached / m_ramtotal;
    drawSection(p, m_ramCachedColour, QRectF(0, contentsRect.top(), 1.0, height));

    height = contentsRect.height() * m_rambuffers / m_ramtotal;
    drawSection(p, m_ramBuffersColour, QRectF(0, contentsRect.top(), 1.0, height));

    height = contentsRect.height() * m_ramapps / m_ramtotal;
    drawSection(p, m_ramUserColour, QRectF(0, contentsRect.top(), 1.0, height));

    p->restore();
}

void SystemLoadViewer::paintInterface(QPainter *p, const QStyleOptionGraphicsItem *option, const QRect& contentsRect)
{
    p->save();
    p->setRenderHint(QPainter::SmoothPixmapTransform);

    QRect rotatedContentsRect(contentsRect);


    if (!verticalBars() && (formFactor() == Plasma::Horizontal || formFactor() == Plasma::Vertical)) {
        p->rotate(90);
        p->translate(0, -(contentsRect.width() + contentsRect.x() * 2));
        rotatedContentsRect.setWidth(contentsRect.height());
        rotatedContentsRect.setHeight(contentsRect.width());
    }

    const qreal barWidth = rotatedContentsRect.width() / static_cast<qreal>(barCount());

    rotatedContentsRect.setWidth(barWidth);

    p->translate(rotatedContentsRect.left(), 0);
    rotatedContentsRect.moveLeft(0.0);

    if(m_showMultiCPU){
        for (int i = 0; i < cpuCount(); ++i) {
            paintCPUUsage(p, option, rotatedContentsRect, m_cpuInfo[i]);
            p->translate(barWidth, 0);
        }
    } else {
        paintCPUUsage(p, option, rotatedContentsRect, m_systemCpuInfo);
        p->translate(barWidth, 0);
    }

    paintRAMUsage(p, option, rotatedContentsRect);

    if (m_swapAvailable) {
        p->translate(barWidth, 0);
        paintSwapUsage(p, option, rotatedContentsRect);
    }
    p->restore();
}

void SystemLoadViewer::configChanged()
{
    KConfigGroup cg = config();

    setVerticalBars(cg.readEntry("vertical", true));
    m_showMultiCPU = cg.readEntry("show_multiple_cpus", false);
    m_updateInterval = cg.readEntry("update_interval", 2000);

    m_cpuUserColour = QColor(cg.readEntry("colour_cpu_user", QString("#0000FF")));
    m_cpuNiceColour = QColor(cg.readEntry("colour_cpu_nice", QString("#FFFF00")));
    m_cpuDiskColour = QColor(cg.readEntry("colour_cpu_disk", QString("#006400")));
    m_cpuSysColour = QColor(cg.readEntry("colour_cpu_sys", QString("#FF0000")));
    m_ramCachedColour = QColor(cg.readEntry("colour_ram_cached", QString("#006400")));
    m_ramBuffersColour = QColor(cg.readEntry("colour_ram_buffers", QString("#FFFF00")));
    m_ramUserColour = QColor(cg.readEntry("colour_ram_used", QString("#0000FF")));
    m_swapUsedColour = QColor(cg.readEntry("colour_swap_used", QString("#00CDCD")));
    m_freeResourceColor = cg.readEntry("colour_free_resource", QString("#808080"));
    m_freeResourceColor.setAlpha(cg.readEntry("transparency_free_resource", 0));
}

void SystemLoadViewer::toolTipAboutToShow()
{
    QString  content = "";
    int      cpuUsage;

    // CPU Usage
    if (m_showMultiCPU) {
        for (uint i = 0; i < m_numCPUs; ++i) {
            cpuUsage = qRound((m_cpuInfo[i].nice + m_cpuInfo[i].user + m_cpuInfo[i].disk + m_cpuInfo[i].sys) * 100);
            if (m_cpuInfo[i].clockValid) {
                content += i18n("CPU %1 Usage: %2% at %3 MHz<br />", i+1, cpuUsage, m_cpuInfo[i].clock);
            }
            else {
                content += i18n("CPU %1 Usage: %2%<br />", i+1, cpuUsage);
            }
        }
    } else {
        cpuUsage = qRound((m_systemCpuInfo.nice + m_systemCpuInfo.user + m_systemCpuInfo.disk + m_systemCpuInfo.sys) * 100);
        if (m_systemCpuInfo.clockValid) {
            if (m_numCPUs >1) {
                content += i18n("CPU Usage: %1% at %2 MHz/CPU<br />", cpuUsage, m_systemCpuInfo.clock);
            }
            else {
                content += i18n("CPU Usage: %1% at %2 MHz<br />", cpuUsage, m_systemCpuInfo.clock);
            }
        }
        else {
            content += i18n("CPU Usage: %1%<br />", cpuUsage);
        }
    }

    // Memory Usage
    const int ramUsage = qRound((m_ramapps / m_ramtotal) * 100);
    int ramTotalMiB = qRound(m_ramtotal / 1024);
    content += i18n("Ram Usage: %1% of %2 MiB<br />", ramUsage, ramTotalMiB);

    // Swap Usage
    if (m_swapAvailable) {
        const int swapUsage = qRound((m_swapused / m_swaptotal) * 100);
        int swapTotalMiB = qRound(m_swaptotal / 10.24); //m_swaptotal is already divided by 100
        content += i18n("Swap Usage: %1% of %2 MiB", swapUsage, swapTotalMiB);
    } else {
        content += i18nc("not available means the system does not have swap", "Swap: not available");
    }
        
    Plasma::ToolTipManager::self()->setContent(this, Plasma::ToolTipContent(i18n("System Load Viewer"), content, KIcon("view-statistics")));
}

bool SystemLoadViewer::verticalBars() const
{
    return m_barOrientation == Qt::Vertical;
}

void SystemLoadViewer::setVerticalBars(bool verticalOrientation)
{
    m_barOrientation = verticalOrientation ? Qt::Vertical : Qt::Horizontal;
}
