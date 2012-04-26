/***************************************************************************
 *   Copyright (C) 2007 by Robert Knight <robertknight@gmail.com>          *
 *   Copyright (C) 2008 by Alexis Ménard <darktears31@gmail.com>           *
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

#include "tooltips/tooltipmanager.h"
// Own
#include "tasks.h"
#include "windowtaskitem.h"
#include "taskgroupitem.h"
#include "jobmanager.h"
#include "dockmanager.h"
#include "mediabuttons.h"
#include "unity.h"
#include "recentdocuments.h"

//Taskmanager
#include "taskmanager/groupmanager.h"
#include "taskmanager/taskgroup.h"
#include "taskmanager/taskitem.h"

// KDE
#include <KConfigDialog>
#include <KDebug>
#include <KStandardDirs>

// Qt
#include <QGraphicsScene>
#include <QGraphicsLinearLayout>
#include <QVariant>
#include <QBuffer>
#include <QIODevice>
#include <QFile>

// Plasma
#include <Plasma/Containment>
#include <Plasma/FrameSvg>
#include <Plasma/Theme>
#include <Plasma/WindowEffects>

static void renameConfig()
{
    // Rename pre0.9 taskmanagerrc to taskmanagerrulesrc
    QString oldName = KStandardDirs::locateLocal("config", "taskmanagerrc");

    if (QFile::exists(oldName)) {
        QString newName = KStandardDirs::locateLocal("config", "taskmanagerrulesrc");

        if (QFile::exists(newName)) {
            QFile::remove(oldName);
        } else {
            QFile::rename(oldName, newName);
        }
    }
}

static void setCurrentIndex(QComboBox *combo, int val)
{
    for (int i = 0; i < combo->count(); ++i) {
        if (combo->itemData(i).toInt() == val) {
            combo->setCurrentIndex(i);
            break;
        }
    }
}

class GroupManager : public TaskManager::GroupManager
{
public:
    GroupManager(Plasma::Applet *applet)
        : TaskManager::GroupManager(applet),
          m_applet(applet) {
        setGroupingStrategy(GroupManager::ProgramGrouping);
        setSortingStrategy(GroupManager::ManualSorting);
        setShowOnlyCurrentActivity(true);
        setShowOnlyCurrentDesktop(true);
        setShowOnlyCurrentScreen(false);
        setShowOnlyMinimized(false);
        setOnlyGroupWhenFull(false);
        setSeparateLaunchers(false);
        setForceGrouping(true);
        readLauncherConfig();
    }

protected:
    KConfigGroup config() const {
        return m_applet->config();
    }

private:
    Plasma::Applet *m_applet;
};

static const int constMinSpacing = 0;
static const int constMaxSpacing = 50;
static const int constMinIconScale = 49;
static const int constMaxIconScale = 100;

Tasks::Tasks(QObject* parent, const QVariantList &arguments)
    : Plasma::Applet(parent, arguments),
      m_toolTips(TT_Instant),
      m_highlightWindows(true),
      m_launcherIcons(false),
      m_groupClick(GC_PresentWindows),
      m_rotate(false),
      m_style(Style_Plasma),
      m_showSeparator(Sep_WhenNeeded),
      m_middleClick(MC_NewInstance),
      m_spacing(0),
      m_iconScale(constMinIconScale), // constMinIconScale==automatic scaling!!!
      m_taskItemBackground(0),
      m_progressBar(0),
      m_badgeBackground(0),
      m_indicators(0),
      m_leftMargin(0),
      m_topMargin(0),
      m_rightMargin(0),
      m_bottomMargin(0),
      m_offscreenLeftMargin(0),
      m_offscreenTopMargin(0),
      m_offscreenRightMargin(0),
      m_offscreenBottomMargin(0),
      m_rootGroupItem(0),
      m_groupManager(0),
      m_lockAct(0),
      m_unlockAct(0),
      m_refreshAct(0)
{
    KGlobal::locale()->insertCatalog("icontasks");
    renameConfig();
    setHasConfigurationInterface(true);
    setAspectRatioMode(Plasma::IgnoreAspectRatio);
    m_screenTimer.setSingleShot(true);
    m_screenTimer.setInterval(300);
    resize(500, 58);

    setAcceptDrops(true);
}

Tasks::~Tasks()
{
    JobManager::self()->setEnabled(false);
    DockManager::self()->setEnabled(false);
    MediaButtons::self()->setEnabled(false);
    Unity::self()->setEnabled(false);
    RecentDocuments::self()->setEnabled(false);
    delete m_rootGroupItem;
    delete m_groupManager;
    AbstractTaskItem::clearCaches();
}

void Tasks::init()
{
    m_groupManager = new GroupManager(this);
    Plasma::Containment* appletContainment = containment();
    if (appletContainment) {
        m_groupManager->setScreen(appletContainment->screen());
    }

    connect(m_groupManager, SIGNAL(reload()), this, SLOT(reload()));
    connect(m_groupManager, SIGNAL(configChanged()), this, SIGNAL(configNeedsSaving()));

    m_rootGroupItem = new TaskGroupItem(this, this);
    m_rootGroupItem->expand();
    m_rootGroupItem->setGroup(m_groupManager->rootGroup());

    connect(m_rootGroupItem, SIGNAL(sizeHintChanged(Qt::SizeHint)), this, SLOT(changeSizeHint(Qt::SizeHint)));

    setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));
    setMaximumSize(INT_MAX, INT_MAX);

    layout = new QGraphicsLinearLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));
    layout->setMaximumSize(INT_MAX, INT_MAX);
    layout->setOrientation(Qt::Vertical);
    layout->addItem(m_rootGroupItem);
    setLayout(layout);

    configChanged();
    if (containment()) {
        IconTasks::ToolTipManager::self()->setCorona(containment()->corona());
    }
}

void Tasks::configChanged()
{
    KConfigGroup cg = config();
    bool changed = false;

    // only update these if they have actually changed, because they make the
    // group manager reload its tasks list
    const bool showOnlyCurrentDesktop = cg.readEntry("showOnlyCurrentDesktop", m_groupManager->showOnlyCurrentDesktop());
    if (showOnlyCurrentDesktop != m_groupManager->showOnlyCurrentDesktop()) {
        m_groupManager->setShowOnlyCurrentDesktop(showOnlyCurrentDesktop);
        changed = true;
    }

    const bool showOnlyCurrentActivity = cg.readEntry("showOnlyCurrentActivity", m_groupManager->showOnlyCurrentActivity());
    if (showOnlyCurrentActivity != m_groupManager->showOnlyCurrentActivity()) {
        m_groupManager->setShowOnlyCurrentActivity(showOnlyCurrentActivity);
        changed = true;
    }

    const bool showOnlyCurrentScreen = cg.readEntry("showOnlyCurrentScreen", m_groupManager->showOnlyCurrentScreen());
    if (showOnlyCurrentScreen != m_groupManager->showOnlyCurrentScreen()) {
        m_groupManager->setShowOnlyCurrentScreen(showOnlyCurrentScreen);
        changed = true;
    }

    TaskManager::GroupManager::TaskSortingStrategy sortingStrategy =
        static_cast<TaskManager::GroupManager::TaskSortingStrategy>(
            cg.readEntry("sortingStrategy",
                         static_cast<int>(m_groupManager->sortingStrategy()))
        );

    if (sortingStrategy != m_groupManager->sortingStrategy()) {
        m_groupManager->setSortingStrategy(sortingStrategy);
        changed = true;
    }

    const int maxRows = cg.readEntry("maxRows", m_rootGroupItem->maxRows());
    if (maxRows != m_rootGroupItem->maxRows()) {
        m_rootGroupItem->setMaxRows(maxRows);
        changed = true;
    }

    const bool launcherIcons = cg.readEntry("launcherIcons", m_launcherIcons);
    if (launcherIcons != m_launcherIcons) {
        m_launcherIcons = launcherIcons;
        changed = true;
    }

    const GroupClick groupClick = static_cast<GroupClick>(cg.readEntry("groupClick", static_cast<int>(m_groupClick)));
    if (groupClick != m_groupClick) {
        m_groupClick = groupClick;
        changed = true;
    }

    const bool rotate = cg.readEntry("rotate", m_rotate);
    if (rotate != m_rotate) {
        m_rotate = rotate;
        changed = true;
    }

    const Style style = static_cast<Style>(cg.readEntry("style", static_cast<int>(m_style)));
    if (style != m_style) {
        m_style = style;
        changed = true;
    }

    if (Style_IconTasksColored != style) {
        AbstractTaskItem::clearCaches(AbstractTaskItem::Cache_Bgnd);
    }

    const bool showProgress = cg.readEntry("showProgress", true);
    if (showProgress != JobManager::self()->isEnabled()) {
        changed = true;
    }
    JobManager::self()->setEnabled(showProgress);

    const bool mediaButtons = cg.readEntry("mediaButtons", true);
    if (mediaButtons != MediaButtons::self()->isEnabled()) {
        changed = true;
    }
    MediaButtons::self()->setEnabled(mediaButtons);

    const bool unity = cg.readEntry("unity", true);
    if (unity != Unity::self()->isEnabled()) {
        changed = true;
    }
    Unity::self()->setEnabled(unity);

    const bool recentDocuments = cg.readEntry("recentDocuments", true);
    if (recentDocuments != RecentDocuments::self()->isEnabled()) {
        changed = true;
    }
    RecentDocuments::self()->setEnabled(recentDocuments);

    const SeparatorType showSeparator = TaskManager::GroupManager::ManualSorting == m_groupManager->sortingStrategy()
                                        ? static_cast<SeparatorType>(cg.readEntry("showSeparator", static_cast<int>(m_showSeparator)))
                                        : Sep_Never;
    if (showSeparator != m_showSeparator) {
        m_showSeparator = showSeparator;
        changed = true;
    }

    const MiddleClick middleClick = static_cast<MiddleClick>(cg.readEntry("middleClick", static_cast<int>(m_middleClick)));
    if (middleClick != m_middleClick) {
        m_middleClick = middleClick;
        changed = true;
    }

    const int spacing = cg.readEntry("spacing", m_spacing);
    if (spacing != m_spacing && spacing >= constMinSpacing && spacing <= constMaxSpacing) {
        m_spacing = spacing;
        changed = true;
        m_rootGroupItem->relayoutItems();
    }

    const int iconScale = cg.readEntry("iconScale", m_iconScale);
    if (iconScale != m_iconScale && iconScale >= constMinIconScale && iconScale <= constMaxIconScale) {
        m_iconScale = iconScale;
        changed = true;
    }

    if (autoIconScaling()) {
        AbstractTaskItem::clearCaches(AbstractTaskItem::Cache_Scale);
    }

    const int toolTips = cg.readEntry("toolTips", (int)m_toolTips);
    if (toolTips != (int)m_toolTips && toolTips >= TT_None && toolTips <= TT_Delayed) {
        m_toolTips = (TT_Type)toolTips;
        changed = true;
    }

    const bool highlightWindows = cg.readEntry("highlightWindows", true);
    if (highlightWindows != m_highlightWindows) {
        m_highlightWindows = highlightWindows;
        changed = true;
    }

    const int previewSize = cg.readEntry("previewSize", IconTasks::ToolTipManager::self()->previewWidth());
    if (previewSize != IconTasks::ToolTipManager::self()->previewWidth() &&
            previewSize >= IconTasks::ToolTipManager::MIN_PREVIEW_SIZE &&
            previewSize <= IconTasks::ToolTipManager::MAX_PREVIEW_SIZE) {
        IconTasks::ToolTipManager::self()->setPreviewSize(previewSize);
    }

    DockManager::self()->readConfig(cg);

    // If we have not already read the launchers, then try now...
    // ...this is mainly required for plasmoidviewer...
    if (0 == m_groupManager->launcherCount()) {
        m_groupManager->readLauncherConfig();
    }

    const bool launchersLocked = groupManager().launcherCount()
                                    ? cg.readEntry("launchersLocked", m_groupManager->launchersLocked())
                                    : false;

    if (launchersLocked!=m_groupManager->launchersLocked()) {
        m_groupManager->setLaunchersLocked(launchersLocked);
        changed = true;
    }

    if (changed) {
        emit settingsChanged();
        update();
    }
}

void Tasks::reload()
{
    TaskGroup *newGroup = m_groupManager->rootGroup();
    if (newGroup != m_rootGroupItem->abstractItem()) {
        m_rootGroupItem->setGroup(newGroup);
    } else {
        m_rootGroupItem->reload();
    }
}

TaskManager::GroupManager &Tasks::groupManager() const
{
    return *m_groupManager;
}

void Tasks::constraintsEvent(Plasma::Constraints constraints)
{
    if (m_groupManager && constraints & Plasma::ScreenConstraint) {
        Plasma::Containment* appletContainment = containment();
        if (appletContainment) {
            m_groupManager->setScreen(appletContainment->screen());
        }
    }

    if (constraints & Plasma::LocationConstraint) {
        QTimer::singleShot(500, this, SLOT(publishIconGeometry()));
    }

    setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));
    emit constraintsChanged(constraints);
}

void Tasks::publishIconGeometry()
{
    foreach (AbstractTaskItem * item, m_rootGroupItem->members()) {
        item->publishIconGeometry();
    }
}

Plasma::FrameSvg* Tasks::itemBackground()
{
    if (!m_taskItemBackground) {
        m_taskItemBackground = new Plasma::FrameSvg(this);
        m_taskItemBackground->setImagePath("widgets/tasks");
        m_taskItemBackground->setCacheAllRenderedFrames(true);
    }

    return m_taskItemBackground;
}

Plasma::FrameSvg* Tasks::progressBar()
{
    if (!m_progressBar) {
        m_progressBar = new Plasma::FrameSvg(this);
        m_progressBar->setImagePath("icontasks/progress");
        m_progressBar->setCacheAllRenderedFrames(true);
    }

    return m_progressBar;
}

Plasma::FrameSvg* Tasks::badgeBackground()
{
    if (!m_badgeBackground) {
        m_badgeBackground = new Plasma::FrameSvg(this);
        m_badgeBackground->setImagePath("icontasks/badge");
        m_badgeBackground->setCacheAllRenderedFrames(true);
    }

    return m_badgeBackground;
}

Plasma::Svg* Tasks::indicators()
{
    if (!m_indicators) {
        m_indicators = new Plasma::Svg(this);
        m_indicators->setImagePath("icontasks/indicators");
        m_indicators->setContainsMultipleImages(true);
    }

    return m_indicators;
}

void Tasks::resizeItemBackground(const QSizeF &size)
{
    if (!m_taskItemBackground) {
        itemBackground();
    }

    if (m_taskItemBackground->frameSize() == size) {
        return;
    }

    m_taskItemBackground->resizeFrame(size);

    QString oldPrefix = m_taskItemBackground->prefix();
    m_taskItemBackground->setElementPrefix("normal");
    //get the margins now
    m_taskItemBackground->getMargins(m_leftMargin, m_topMargin, m_rightMargin, m_bottomMargin);

    // the offscreen margins are always whatever the svg naturally is
    m_offscreenLeftMargin = m_leftMargin;
    m_offscreenTopMargin = m_topMargin;
    m_offscreenRightMargin = m_rightMargin;
    m_offscreenBottomMargin = m_bottomMargin;

    //if the task height is too little shrink the top and bottom margins
    if (size.height() - m_topMargin - m_bottomMargin < KIconLoader::SizeSmall) {
        m_topMargin = m_bottomMargin = qMax(1, int((size.height() - KIconLoader::SizeSmall) / 2));
    }
    m_taskItemBackground->setElementPrefix(oldPrefix);
}

void Tasks::resizeProgressBar(const QSizeF &size)
{
    if (!m_progressBar) {
        progressBar();
    }

    if (m_progressBar->frameSize() == size) {
        return;
    }

    m_progressBar->resizeFrame(size);
}

void Tasks::resizeBadgeBackground(const QSizeF &size)
{
    if (!m_badgeBackground) {
        badgeBackground();
    }

    if (m_badgeBackground->frameSize() == size) {
        return;
    }

    m_badgeBackground->resizeFrame(size);
}

QSizeF Tasks::sizeHint(Qt::SizeHint which, const QSizeF &constraint) const
{
    if (m_rootGroupItem && which == Qt::PreferredSize) {
        return m_rootGroupItem->preferredSize();
    } else {
        return Plasma::Applet::sizeHint(which, constraint);
    }
}

void Tasks::changeSizeHint(Qt::SizeHint which)
{
    emit sizeHintChanged(which);
}

void Tasks::updateShowSeparator()
{
    bool enableShowSep = 1 == m_appUi.maxRows->value() &&
                         TaskManager::GroupManager::ManualSorting == m_appUi.sortingStrategy->itemData(m_appUi.sortingStrategy->currentIndex()).toInt();

    m_appUi.showSeparator->setEnabled(enableShowSep);
    m_appUi.showSeparator_label->setEnabled(enableShowSep);
    if (!enableShowSep) {
        setCurrentIndex(m_appUi.showSeparator, (int)Sep_Never);
    }
}

void Tasks::toolTipsModified()
{
    m_appUi.previewSize->setEnabled(TT_None != m_appUi.toolTips->itemData(m_appUi.toolTips->currentIndex()).toInt());
    m_appUi.previewSize_label->setEnabled(TT_None != m_appUi.toolTips->itemData(m_appUi.toolTips->currentIndex()).toInt());
}

void Tasks::styleModified()
{
    m_appUi.rotate->setEnabled(Style_Plasma == m_appUi.style->itemData(m_appUi.style->currentIndex()).toInt());
    if (Style_Plasma != m_appUi.style->itemData(m_appUi.style->currentIndex()).toInt()) {
        m_appUi.rotate->setChecked(false);
    }
    m_appUi.rotate_label->setEnabled(Style_Plasma == m_appUi.style->itemData(m_appUi.style->currentIndex()).toInt());
}

void Tasks::refresh()
{
    QWidget *rw = new QWidget;
    rw->show();
    QTimer::singleShot(25, rw, SLOT(deleteLater()));
}

void Tasks::createConfigurationInterface(KConfigDialog *parent)
{
    QWidget *appearance = new QWidget;
    QWidget *behaviour = new QWidget;
    m_appUi.setupUi(appearance);
    m_behaviourUi.setupUi(behaviour);
    connect(parent, SIGNAL(applyClicked()), this, SLOT(configAccepted()));
    connect(parent, SIGNAL(okClicked()), this, SLOT(configAccepted()));
    parent->addPage(appearance, i18n("Appearance"), "preferences-desktop-theme");
    parent->addPage(behaviour, i18n("Behaviour"), "system-run");

    m_appUi.toolTips->addItem(i18n("Do Not Show"), QVariant(TT_None));
    m_appUi.toolTips->addItem(i18n("Show Immediately"), QVariant(TT_Instant));
    m_appUi.toolTips->addItem(i18n("Show After Delay"), QVariant(TT_Delayed));
    connect(m_appUi.toolTips, SIGNAL(currentIndexChanged(int)), parent, SLOT(settingsModified()));
    connect(m_appUi.toolTips, SIGNAL(currentIndexChanged(int)), this, SLOT(toolTipsModified()));
    setCurrentIndex(m_appUi.toolTips, (int)m_toolTips);
    m_appUi.highlightWindows->setChecked(m_highlightWindows);
    m_appUi.launcherIcons->setChecked(m_launcherIcons);
    m_behaviourUi.groupClick->addItem(i18n("Minimize/Restore"), QVariant(GC_MinMax));
    m_behaviourUi.groupClick->addItem(i18n("Present Windows Effect"), QVariant(GC_PresentWindows));
    m_behaviourUi.groupClick->addItem(i18n("Show Popup Menu"), QVariant(GC_Popup));
    setCurrentIndex(m_behaviourUi.groupClick, m_groupClick);
    m_appUi.rotate->setChecked(m_rotate);
    m_appUi.style->addItem(i18n("Use Workspace Theme"), QVariant(Style_Plasma));
    m_appUi.style->addItem(i18n("Use Indicators"), QVariant(Style_IconTasks));
    m_appUi.style->addItem(i18n("Use Indicators & Colored Background"), QVariant(Style_IconTasksColored));
    m_appUi.style->setCurrentIndex((int)m_style);
    m_appUi.showSeparator->addItem(i18n("Never Show"), QVariant(Sep_Never));
    m_appUi.showSeparator->addItem(i18n("Show When Required"), QVariant(Sep_WhenNeeded));
    m_appUi.showSeparator->addItem(i18n("Always Show"), QVariant(Sep_Always));
    m_appUi.showSeparator->setCurrentIndex((int)m_showSeparator);
    m_behaviourUi.middleClick->addItem(i18n("Start New Instance"), QVariant(MC_NewInstance));
    m_behaviourUi.middleClick->addItem(i18n("Close Application"), QVariant(MC_Close));
    m_behaviourUi.middleClick->addItem(i18n("Move To Current Desktop"), QVariant(MC_MoveToCurrentDesktop));
    m_behaviourUi.middleClick->addItem(i18n("Nothing"), QVariant(MC_None));
    m_behaviourUi.middleClick->setCurrentIndex((int)m_middleClick);
    setCurrentIndex(m_behaviourUi.middleClick, (int)m_middleClick);
    m_behaviourUi.showProgress->setChecked(JobManager::self()->isEnabled());
    m_behaviourUi.mediaButtons->setChecked(MediaButtons::self()->isEnabled());
    m_behaviourUi.unity->setChecked(Unity::self()->isEnabled());
    m_behaviourUi.recentDocuments->setChecked(RecentDocuments::self()->isEnabled());
    m_appUi.spacing->setRange(constMinSpacing, constMaxSpacing);
    m_appUi.spacing->setValue(m_spacing);
    m_appUi.previewSize->setRange(IconTasks::ToolTipManager::MIN_PREVIEW_SIZE, IconTasks::ToolTipManager::MAX_PREVIEW_SIZE);
    m_appUi.previewSize->setValue(IconTasks::ToolTipManager::self()->previewWidth());
    m_appUi.previewSize->setSingleStep(50);
    m_appUi.iconScale->setRange(constMinIconScale, constMaxIconScale);
    m_appUi.iconScale->setSingleStep(5);
    m_appUi.iconScale->setValue(m_iconScale);
    m_appUi.iconScale->setSpecialValueText(i18n("Automatic"));
    m_behaviourUi.showOnlyCurrentDesktop->setChecked(m_groupManager->showOnlyCurrentDesktop());
    m_behaviourUi.showOnlyCurrentActivity->setChecked(m_groupManager->showOnlyCurrentActivity());
    m_behaviourUi.showOnlyCurrentScreen->setChecked(m_groupManager->showOnlyCurrentScreen());

    m_appUi.sortingStrategy->addItem(i18n("Manually"), QVariant(TaskManager::GroupManager::ManualSorting));
    m_appUi.sortingStrategy->addItem(i18n("Alphabetically"), QVariant(TaskManager::GroupManager::AlphaSorting));
    m_appUi.sortingStrategy->addItem(i18n("By Desktop"), QVariant(TaskManager::GroupManager::DesktopSorting));
    setCurrentIndex(m_appUi.sortingStrategy, (int)m_groupManager->sortingStrategy());
    m_appUi.maxRows->setValue(m_rootGroupItem->maxRows());

    connect(m_appUi.toolTips, SIGNAL(currentIndexChanged(int)), parent, SLOT(settingsModified()));
    connect(m_appUi.highlightWindows, SIGNAL(toggled(bool)), parent, SLOT(settingsModified()));
    connect(m_appUi.launcherIcons, SIGNAL(toggled(bool)), parent, SLOT(settingsModified()));
    connect(m_behaviourUi.groupClick, SIGNAL(currentIndexChanged(int)), parent, SLOT(settingsModified()));
    connect(m_appUi.rotate, SIGNAL(toggled(bool)), parent, SLOT(settingsModified()));
    connect(m_appUi.style, SIGNAL(currentIndexChanged(int)), this, SLOT(styleModified()));
    connect(m_appUi.style, SIGNAL(currentIndexChanged(int)), parent, SLOT(settingsModified()));
    connect(m_appUi.showSeparator, SIGNAL(currentIndexChanged(int)), parent, SLOT(settingsModified()));
    connect(m_behaviourUi.middleClick, SIGNAL(currentIndexChanged(int)), parent, SLOT(settingsModified()));
    connect(m_behaviourUi.showProgress, SIGNAL(toggled(bool)), parent, SLOT(settingsModified()));
    connect(m_behaviourUi.mediaButtons, SIGNAL(toggled(bool)), parent, SLOT(settingsModified()));
    connect(m_behaviourUi.unity, SIGNAL(toggled(bool)), parent, SLOT(settingsModified()));
    connect(m_appUi.spacing, SIGNAL(valueChanged(int)), parent, SLOT(settingsModified()));
    connect(m_appUi.previewSize, SIGNAL(valueChanged(int)), parent, SLOT(settingsModified()));
    connect(m_appUi.iconScale, SIGNAL(valueChanged(int)), parent, SLOT(settingsModified()));
    connect(m_appUi.maxRows, SIGNAL(valueChanged(int)), SLOT(updateShowSeparator()));
    connect(m_appUi.maxRows, SIGNAL(valueChanged(int)), parent, SLOT(settingsModified()));
    connect(m_appUi.sortingStrategy, SIGNAL(currentIndexChanged(int)), SLOT(updateShowSeparator()));
    connect(m_behaviourUi.showOnlyCurrentScreen, SIGNAL(toggled(bool)), parent, SLOT(settingsModified()));
    connect(m_behaviourUi.showOnlyCurrentDesktop, SIGNAL(toggled(bool)), parent, SLOT(settingsModified()));
    connect(m_behaviourUi.showOnlyCurrentActivity, SIGNAL(toggled(bool)), parent, SLOT(settingsModified()));
    m_appUi.launcherIcons_label->setToolTip(m_appUi.launcherIcons->toolTip());
    m_behaviourUi.groupClick_label->setToolTip(m_behaviourUi.groupClick->toolTip());
    m_appUi.rotate_label->setToolTip(m_appUi.rotate->toolTip());
    m_appUi.style_label->setToolTip(m_appUi.style->toolTip());
    m_appUi.maxRows_label->setToolTip(m_appUi.maxRows->toolTip());
    m_appUi.spacing_label->setToolTip(m_appUi.spacing->toolTip());
    m_appUi.previewSize_label->setToolTip(m_appUi.previewSize->toolTip());
    m_appUi.iconScale_label->setToolTip(m_appUi.iconScale->toolTip());
    m_appUi.showSeparator_label->setToolTip(m_appUi.showSeparator->toolTip());
    m_behaviourUi.showProgress_label->setToolTip(m_behaviourUi.showProgress->toolTip());
    m_behaviourUi.mediaButtons_label->setToolTip(m_behaviourUi.mediaButtons->toolTip());
    m_behaviourUi.unity_label->setToolTip(m_behaviourUi.unity->toolTip());
    m_behaviourUi.middleClick_label->setToolTip(m_behaviourUi.middleClick->toolTip());

    updateShowSeparator();
    toolTipsModified();
    styleModified();
    DockManager::self()->addConfigWidget(parent);
    m_groupManager->createConfigurationInterface(parent);
    parent->resize(640, 480);
}

void Tasks::configAccepted()
{
    // just write the config here, and it will get applied in configChanged(),
    // which is called after this when the config dialog is accepted
    KConfigGroup cg = config();

    cg.writeEntry("showOnlyCurrentDesktop", m_behaviourUi.showOnlyCurrentDesktop->isChecked());
    cg.writeEntry("showOnlyCurrentActivity", m_behaviourUi.showOnlyCurrentActivity->isChecked());
    cg.writeEntry("showOnlyCurrentScreen", m_behaviourUi.showOnlyCurrentScreen->isChecked());
    cg.writeEntry("sortingStrategy", m_appUi.sortingStrategy->itemData(m_appUi.sortingStrategy->currentIndex()).toInt());
    cg.writeEntry("maxRows", m_appUi.maxRows->value());
    cg.writeEntry("launcherIcons", m_appUi.launcherIcons->checkState() == Qt::Checked);
    cg.writeEntry("groupClick", m_behaviourUi.groupClick->itemData(m_behaviourUi.groupClick->currentIndex()).toInt());
    cg.writeEntry("rotate", m_appUi.rotate->checkState() == Qt::Checked);
    cg.writeEntry("style", m_appUi.style->itemData(m_appUi.style->currentIndex()).toInt());
    cg.writeEntry("showSeparator", m_appUi.showSeparator->itemData(m_appUi.showSeparator->currentIndex()).toInt());
    cg.writeEntry("middleClick", m_behaviourUi.middleClick->itemData(m_behaviourUi.middleClick->currentIndex()).toInt());
    cg.writeEntry("showProgress", m_behaviourUi.showProgress->checkState() == Qt::Checked);
    cg.writeEntry("mediaButtons", m_behaviourUi.mediaButtons->checkState() == Qt::Checked);
    cg.writeEntry("unity", m_behaviourUi.unity->checkState() == Qt::Checked);
    cg.writeEntry("recentDocuments", m_behaviourUi.recentDocuments->checkState() == Qt::Checked);
    cg.writeEntry("spacing", m_appUi.spacing->value());
    cg.writeEntry("previewSize", m_appUi.previewSize->value());
    cg.writeEntry("iconScale", m_appUi.iconScale->value());
    cg.writeEntry("toolTips", m_appUi.toolTips->itemData(m_appUi.toolTips->currentIndex()).toInt());
    cg.writeEntry("highlightWindows", m_appUi.highlightWindows->checkState() == Qt::Checked);
    DockManager::self()->writeConfig(cg);

    emit configNeedsSaving();
}

bool Tasks::showToolTip() const
{
    return TT_None != m_toolTips;
}

bool Tasks::instantToolTip() const
{
    return TT_Instant == m_toolTips;
}

bool Tasks::autoIconScaling() const
{
    return constMinIconScale == m_iconScale;
}

bool Tasks::highlightWindows() const
{
    return m_highlightWindows;
}

QList<QAction*> Tasks::contextualActions()
{
    QList<QAction*> actionList;
    if (groupManager().launcherCount() && !groupManager().separateLaunchers() &&
            TaskManager::GroupManager::ManualSorting == groupManager().sortingStrategy()) {
        if (groupManager().launchersLocked()) {
            if (!m_unlockAct) {
                m_unlockAct = new QAction(KIcon("object-unlocked"), i18n("Unlock Launchers"), this);
                connect(m_unlockAct, SIGNAL(triggered(bool)), this, SLOT(unlockLaunchers()));
            }
            actionList.append(m_unlockAct);
        } else {
            if (!m_lockAct) {
                m_lockAct = new QAction(KIcon("object-locked"), i18n("Lock Launchers"), this);
                connect(m_lockAct, SIGNAL(triggered(bool)), this, SLOT(lockLaunchers()));
            }
            actionList.append(m_lockAct);
        }
    }

    if (!m_refreshAct) {
        m_refreshAct = new QAction(KIcon("view-refresh"), i18n("Refresh"), this);
        connect(m_refreshAct, SIGNAL(triggered(bool)), this, SLOT(refresh()));
    }
    actionList.append(m_refreshAct);
    return actionList;
}

void Tasks::lockLaunchers()
{
    if (!m_groupManager->launchersLocked()) {
        m_groupManager->setLaunchersLocked(true);
        config().writeEntry("launchersLocked", m_groupManager->launchersLocked());
        emit configNeedsSaving();
    }
}

void Tasks::unlockLaunchers()
{
    if (m_groupManager->launchersLocked()) {
        m_groupManager->setLaunchersLocked(false);
        config().writeEntry("launchersLocked", m_groupManager->launchersLocked());
        emit configNeedsSaving();
    }
}

void Tasks::needsVisualFocus(bool focus)
{
    if (focus) {
        setStatus(Plasma::NeedsAttentionStatus);
    } else {
        foreach (AbstractTaskItem * item, m_rootGroupItem->members()) {
            if (item->taskFlags() & AbstractTaskItem::TaskWantsAttention) {
                // not time to go passive yet! :)
                return;
            }
        }
        setStatus(Plasma::PassiveStatus);
    }
}

TaskGroupItem* Tasks::rootGroupItem()
{
    return m_rootGroupItem;
}

QWidget *Tasks::popupDialog() const
{
    return m_popupDialog.data();
}

bool Tasks::isPopupShowing() const
{
    return m_popupDialog || m_rootGroupItem->windowPreviewOpen();
}

void Tasks::setPopupDialog(bool status)
{
    Q_UNUSED(status)
    QWidget *widget = qobject_cast<QWidget *>(sender());

    if (status && widget->isVisible()) {
        m_popupDialog = widget;
    } else if (m_popupDialog.data() == widget) {
        m_popupDialog.clear();
    }
}

K_EXPORT_PLASMA_APPLET(icontasks, Tasks)

#include "tasks.moc"
