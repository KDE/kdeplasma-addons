
/*
*   Copyright 2010 by Christian Tacke <lordbluelight@gmx.de>
*
*   This program is free software; you can redistribute it and/or modify
*   it under the terms of the GNU Library General Public License as
*   published by the Free Software Foundation; either version 2, or
*   (at your option) any later version.
*
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details
*
*   You should have received a copy of the GNU Library General Public
*   License along with this program; if not, write to the
*   Free Software Foundation, Inc.,
*   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#include "tabbinggroup.h"

#include <QtGui/QGraphicsLinearLayout>
#include <QtGui/QGraphicsView>
#include <QtGui/QGraphicsSceneDragDropEvent>
#include <QtCore/QTimer>

#include <KDE/KConfigDialog>
#include <KDE/KPushButton>

#include <Plasma/TabBar>
#include <Plasma/PushButton>

TabbingGroup::TabbingGroup(QGraphicsItem *parent, Qt::WindowFlags wFlags)
            : AbstractGroup(parent, wFlags),
              m_tabBar(new Plasma::TabBar(this)),
              m_layout(new QGraphicsLinearLayout(Qt::Horizontal)),
              m_newTab(new Plasma::PushButton(this)),
              m_closeTab(new Plasma::PushButton(this)),
              m_deletingTab(false),
              m_changeTabTimer(new QTimer(this)),
              m_changingTab(-1)
{
    m_tabBar->nativeWidget()->setSelectionBehaviorOnRemove(QTabBar::SelectPreviousTab);
    m_tabBar->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    setLayout(m_layout);

    //TODO: in trunk use these:
    //     m_tabBar->setFirstPositionWidget(m_newTab);
    //     m_tabBar->setLastPositionWidget(m_closeTab);

    QGraphicsLinearLayout *newTabLayout = new QGraphicsLinearLayout(Qt::Vertical);
    newTabLayout->addItem(m_newTab);
    newTabLayout->addStretch();

    QGraphicsLinearLayout *closeTabLayout = new QGraphicsLinearLayout(Qt::Vertical);
    closeTabLayout->addItem(m_closeTab);
    closeTabLayout->addStretch();

    m_layout->addItem(newTabLayout);
    m_layout->addItem(m_tabBar);
    m_layout->addItem(closeTabLayout);
    m_layout->setStretchFactor(m_tabBar, 2);

    m_newTab->setIcon(KIcon("tab-new"));
    m_newTab->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    m_closeTab->setIcon(KIcon("tab-close"));
    m_closeTab->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    m_changeTabTimer->setInterval(500); //should see what is the delay used in other apps
    m_changeTabTimer->setSingleShot(true);

    resize(200, 200);
    setGroupType(AbstractGroup::FreeGroup);
    setHasConfigurationInterface(true);
    onImmutabilityChanged(immutability());

    connect(this, SIGNAL(appletAddedInGroup(Plasma::Applet*,AbstractGroup*)),
            this, SLOT(onAppletAdded(Plasma::Applet*,AbstractGroup*)));
    connect(this, SIGNAL(subGroupAddedInGroup(AbstractGroup*,AbstractGroup*)),
            this, SLOT(onSubGroupAdded(AbstractGroup*, AbstractGroup*)));
    connect(this, SIGNAL(immutabilityChanged(Plasma::ImmutabilityType)),
            this, SLOT(onImmutabilityChanged(Plasma::ImmutabilityType)));
    connect(m_tabBar, SIGNAL(currentChanged(int)), this, SLOT(tabBarIndexChanged(int)));
    connect(m_newTab, SIGNAL(clicked()), this, SLOT(addTab()));
    connect(m_closeTab, SIGNAL(clicked()), this, SLOT(closeTab()));
    connect(m_changeTabTimer, SIGNAL(timeout()), this, SLOT(changeTab()));
}

TabbingGroup::~TabbingGroup()
{
}

void TabbingGroup::init()
{
    KConfigGroup group = config();

    QStringList tabs = group.readEntry("Tabs", QStringList());

    if (tabs.isEmpty()) {
        tabs << "New Tab";
    }

    foreach (const QString &tab, tabs) {
        addTab(tab);
    }

    m_tabBar->setCurrentIndex(group.readEntry("CurrentIndex", 0));
}

void TabbingGroup::onAppletAdded(Plasma::Applet *applet, AbstractGroup *)
{
    applet->installEventFilter(this);
    connect(applet, SIGNAL(appletDestroyed(Plasma::Applet*)),
            this, SLOT(onAppletDestroyed(Plasma::Applet*)));
}

void TabbingGroup::onSubGroupAdded(AbstractGroup *subGroup, AbstractGroup *)
{
    subGroup->installEventFilter(this);
    connect(subGroup, SIGNAL(groupDestroyed(AbstractGroup*)),
            this, SLOT(onGroupDestroyed(AbstractGroup*)));
}

void TabbingGroup::onImmutabilityChanged(Plasma::ImmutabilityType immutability)
{
    m_newTab->setVisible(immutability == Plasma::Mutable);
    m_closeTab->setVisible(immutability == Plasma::Mutable);
}

void TabbingGroup::layoutChild(QGraphicsWidget *child, const QPointF &pos)
{
    Q_UNUSED(pos)

    QGraphicsWidget *w = m_tabWidgets.at(m_tabBar->currentIndex());
    child->setParentItem(w);
    child->setPos(mapToItem(w, child->pos()));

    m_children.insert(child, m_tabBar->currentIndex());
}

QString TabbingGroup::pluginName() const
{
    return QString("tabbing");
}

void TabbingGroup::restoreChildGroupInfo(QGraphicsWidget *child, const KConfigGroup &group)
{
    QPointF pos = group.readEntry("Position", QPointF());
    int index = group.readEntry("TabIndex", -1);

    QGraphicsWidget *w = m_tabWidgets.at(index);
    child->setParentItem(w);
    child->setPos(pos);

    m_children.insert(child, index);
}

void TabbingGroup::saveChildGroupInfo(QGraphicsWidget *child, KConfigGroup group) const
{
    group.writeEntry("Position", child->pos());
    group.writeEntry("TabIndex", m_children.value(child));
}

void TabbingGroup::tabBarIndexChanged(int index)
{
    config().writeEntry("CurrentIndex", index);
    emit configNeedsSaving();
}

void TabbingGroup::createConfigurationInterface(KConfigDialog *parent)
{
    QWidget *widget = new QWidget();
    m_ui.setupUi(widget);
    parent->addPage(widget, i18n("General"), "configure");

    for (int i = 0;i < m_tabBar->count();++i) {
        QListWidgetItem *item = new QListWidgetItem(m_tabBar->tabText(i));
        item->setData(Qt::UserRole, i);
        m_ui.listWidget->addItem(item);
    }

    connect(m_ui.modButton, SIGNAL(clicked()), this, SLOT(configModTab()));
    connect(m_ui.upButton, SIGNAL(clicked()), this, SLOT(configUpTab()));
    connect(m_ui.downButton, SIGNAL(clicked()), this, SLOT(configDownTab()));

    connect(parent, SIGNAL(applyClicked()), this, SLOT(configAccepted()));
    connect(parent, SIGNAL(okClicked()), this, SLOT(configAccepted()));
}

void TabbingGroup::addTab(const QString &name, int pos)
{
    if (pos == -1) { //insert after current position in list
        pos = m_tabBar->count();
    }

    QString tab(name);
    if (tab.isEmpty()) {
        tab = i18n("New Tab");
    }

    QGraphicsWidget *w = new QGraphicsWidget(this);
    m_tabBar->insertTab(pos, tab, w);
    m_tabWidgets << w;

    saveTabs();
}

void TabbingGroup::closeTab(int index)
{
    if (index == -1) {
        index = m_tabBar->currentIndex();
    }

    m_deletingTab = true;
    bool deleteNow = true;

    foreach (Plasma::Applet *applet, applets()) {
        if (m_children.value(applet) == index) {
            deleteNow = false;
            applet->destroy();
        }
    }

    foreach (AbstractGroup *group, subGroups()) {
        if (m_children.value(group) == index) {
            deleteNow = false;
            group->destroy();
        }
    }

    if (deleteNow) {
        deleteTab(index);
    }
}

void TabbingGroup::deleteTab(int index)
{
    //HACK workaround for a bug in 4.4 Plasma::TabBar. Fixed in 4.5 and trunk
    int count = m_tabBar->count();
    if (count > 1) {
        if (index == 0) {
            m_tabBar->setCurrentIndex(1);
        } else {
            m_tabBar->setCurrentIndex(0);
        }
    }
    //end of HACK

    m_tabBar->removeTab(index);
    m_tabWidgets.removeAt(index);

    if (m_tabBar->count() == 0) {
        addTab();
        return;
    }

    saveTabs();

    m_deletingTab = false;
}

void TabbingGroup::onAppletDestroyed(Plasma::Applet *applet)
{
    if (!m_deletingTab) {
        m_children.remove(applet);
        return;
    }

    int tab = m_children.value(applet);

    m_children.remove(applet);
    if (m_children.keys(tab).count() == 0) {
        deleteTab(tab);
    }
}

void TabbingGroup::onGroupDestroyed(AbstractGroup *group)
{
    if (!m_deletingTab) {
        m_children.remove(group);
        return;
    }

    int tab = m_children.value(group);

    m_children.remove(group);
    if (m_children.keys(tab).count() == 0) {
        deleteTab(tab);
    }
}

void TabbingGroup::saveTabs()
{
    QStringList tabs;
    for (int i = 0; i < m_tabBar->count(); ++i) {
        tabs << m_tabBar->tabText(i);
    }

    config().writeEntry("Tabs", tabs);

    emit configNeedsSaving();
}

void TabbingGroup::configModTab()
{
    int pos = m_ui.listWidget->currentRow();

    if (pos == -1) {
        return;
    }

    QString title = m_ui.tileEdit->text();

    if (title == QString()) {
        return;
    }

    m_ui.listWidget->item(pos)->setText(title);
}

void TabbingGroup::configUpTab()
{
    int pos = m_ui.listWidget->currentRow();

    if (pos < 1) {
        return;
    }

    QListWidgetItem *item = m_ui.listWidget->takeItem(pos);

    m_ui.listWidget->insertItem(pos - 1, item);
    m_ui.listWidget->setCurrentRow(pos - 1);
}

void TabbingGroup::configDownTab()
{
    int pos = m_ui.listWidget->currentRow();

    if (pos == -1 || pos == m_ui.listWidget->count() - 1) {
        return;
    }

    QListWidgetItem *item = m_ui.listWidget->takeItem(pos);

    m_ui.listWidget->insertItem(pos + 1, item);
    m_ui.listWidget->setCurrentRow(pos + 1);
}

void TabbingGroup::configAccepted()
{
    QStringList tabs;
    QMap<QGraphicsWidget *, int> childrenToBeMoved;
    for (int i = 0; i < m_ui.listWidget->count(); ++i) {
        QListWidgetItem *item = m_ui.listWidget->item(i);
        tabs.insert(i, item->text());

        int from = item->data(Qt::UserRole).toInt();
        m_tabBar->setTabText(i, item->text());

        if (from != i) {
            foreach (QGraphicsWidget *child, children()) {
                if (m_children.value(child) == from) {
                    childrenToBeMoved.insert(child, i);
                }
            }
        }
    }

    if (!childrenToBeMoved.isEmpty()) {
        //reparent children of moved tabs
        QList<QGraphicsWidget *> children = childrenToBeMoved.keys();
        foreach (QGraphicsWidget *child, children) {
            int tab = childrenToBeMoved.value(child);
            child->setParentItem(m_tabBar->tabAt(tab)->graphicsItem());
            m_children.insert(child, tab);
        }

        saveChildren();
    }

    saveTabs();
}

bool TabbingGroup::eventFilter(QObject *obj, QEvent *event)
{
    QGraphicsWidget *w = static_cast<QGraphicsWidget *>(obj);
    if (!children().contains(w)) {
        return false;
    }

    if (event->type() == QEvent::GraphicsSceneMove) {
        QGraphicsView *v = view();
        QPointF pos = m_tabBar->mapFromScene(v->mapToScene(v->mapFromGlobal(QCursor::pos())));

        int index = m_tabBar->nativeWidget()->tabAt(pos.toPoint());
        if (index == -1) {
            m_changingTab = -1;
            m_changeTabTimer->stop();
        } else {
            m_changingTab = index;
            m_changeTabTimer->start();
        }
    }

    return false;
}

void TabbingGroup::dragMoveEvent(QGraphicsSceneDragDropEvent *event)
{
    QPointF pos = m_tabBar->mapFromScene(static_cast<QGraphicsSceneDragDropEvent *>(event)->scenePos());

    int index = m_tabBar->nativeWidget()->tabAt(pos.toPoint());
    if (index == -1) {
        m_changingTab = -1;
        m_changeTabTimer->stop();
    } else {
        m_changingTab = index;
        m_changeTabTimer->start();
    }
}

void TabbingGroup::changeTab()
{
    if (m_changingTab != -1) {
        m_tabBar->setCurrentIndex(m_changingTab);
        m_changingTab = -1;
    }
}

#include "tabbinggroup.moc"
