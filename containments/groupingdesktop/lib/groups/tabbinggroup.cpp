/*
 *   Copyright 2010 by Christian Tacke <lordbluelight@gmx.de>
 *   Copyright 2010 by Giulio Camuffo <giuliocamuffo@gmail.com>
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

#include "groupingcontainment.h"

REGISTER_GROUP(TabbingGroup)

TabbingGroup::TabbingGroup(QGraphicsItem *parent, Qt::WindowFlags wFlags)
            : AbstractGroup(parent, wFlags),
              m_tabBar(new Plasma::TabBar(this)),
              m_layout(new QGraphicsLinearLayout(Qt::Horizontal)),
              m_newTab(new Plasma::PushButton(this)),
              m_closeTab(new Plasma::PushButton(this)),
              m_changeTabTimer(new QTimer(this)),
              m_changingTab(-1),
              m_deletingTab(false)
{
    QList<QGraphicsItem *> items = m_tabBar->childItems();
    foreach (QGraphicsItem *child, items) {
        m_actualTabBar = qgraphicsitem_cast<QGraphicsProxyWidget *>(child);
        if (m_actualTabBar) {
            break;
        }
    }

    m_tabBar->nativeWidget()->setSelectionBehaviorOnRemove(QTabBar::SelectPreviousTab);

    m_tabBar->setFirstPositionWidget(m_newTab);
    m_tabBar->setLastPositionWidget(m_closeTab);

    m_newTab->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);
    m_closeTab->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);

    m_layout->addItem(m_tabBar);
    setLayout(m_layout);

    m_newTab->setIcon(KIcon("tab-new"));
    m_closeTab->setIcon(KIcon("tab-close"));

    m_changeTabTimer->setInterval(500); //should see what is the delay used in other apps
    m_changeTabTimer->setSingleShot(true);

    resize(200, 200);
    setMinimumSize(100, 150);
    setGroupType(AbstractGroup::FreeGroup);
    setHasConfigurationInterface(true);

    connect(this, SIGNAL(subGroupAddedInGroup(AbstractGroup*,AbstractGroup*)),
            this, SLOT(onSubGroupAdded(AbstractGroup*,AbstractGroup*)));
    connect(this, SIGNAL(subGroupRemovedFromGroup(AbstractGroup*,AbstractGroup*)),
            this, SLOT(onSubGroupRemoved(AbstractGroup*,AbstractGroup*)));
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

    m_tabs = group.readEntry("Tabs", QStringList());

    m_tabBar->setCurrentIndex(group.readEntry("CurrentIndex", 0));

    onImmutabilityChanged(immutability());
}

void TabbingGroup::onSubGroupAdded(AbstractGroup *subGroup, AbstractGroup *)
{
    subGroup->installEventFilter(this);
}

void TabbingGroup::onSubGroupRemoved(AbstractGroup *subGroup, AbstractGroup *)
{
    subGroup->removeEventFilter(this);
    if (m_deletingTab) {
        deleteTab(m_tabGroups.indexOf(subGroup));
    }
}

void TabbingGroup::onImmutabilityChanged(Plasma::ImmutabilityType immutability)
{
    m_newTab->setVisible(immutability == Plasma::Mutable);
    m_closeTab->setVisible(immutability == Plasma::Mutable);
}

void TabbingGroup::layoutChild(QGraphicsWidget *child, const QPointF &pos)
{
    Q_UNUSED(pos)

    QGraphicsWidget *w = m_tabGroups.at(m_tabBar->currentIndex());
    child->setParentItem(w);
    child->setPos(mapToItem(w, pos));
}

QString TabbingGroup::pluginName() const
{
    return QString("tabbing");
}

void TabbingGroup::restoreChildren()
{
    AbstractGroup::restoreChildren();

    if (m_tabs.isEmpty()) {
        addTab();
    }
}

void TabbingGroup::restoreChildGroupInfo(QGraphicsWidget *child, const KConfigGroup &group)
{
    int index = group.readEntry("TabIndex", -1);
    QString name = m_tabs.at(index);

    AbstractGroup *g = static_cast<AbstractGroup *>(child);
    g->setIsMainGroup();
    m_tabBar->insertTab(index, name, child);
    m_tabGroups << g;
}

void TabbingGroup::saveChildGroupInfo(QGraphicsWidget *child, KConfigGroup group) const
{
    group.writeEntry("TabIndex", m_tabGroups.indexOf(static_cast<AbstractGroup *>(child)));
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
    parent->addPage(widget, i18nc("a general page in the config dialog", "General"), "configure");

    for (int i = 0; i < m_tabGroups.count(); ++i) {
        QListWidgetItem *item = new QListWidgetItem(m_tabBar->tabText(i));
        item->setData(Qt::UserRole, i);
        m_ui.listWidget->addItem(item);
        m_ui.listWidget->setCurrentItem(item);
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

    AbstractGroup *g = containment()->addGroup("grid");
    addSubGroup(g, false);
    m_tabBar->insertTab(pos, tab, g);
    g->setIsMainGroup();
    m_tabGroups << g;

    m_tabs << tab;
    saveTabs();
}

void TabbingGroup::closeTab(int index)
{
    if (index == -1) {
        index = m_tabBar->currentIndex();
    }

    m_deletingTab = true;

    AbstractGroup *group = m_tabGroups.at(index);
    group->destroy();
}

void TabbingGroup::deleteTab(int index)
{
    m_tabBar->removeTab(index);
    m_tabGroups.removeAt(index);
    m_tabs.removeAt(index);

    m_deletingTab = false;

    if (m_tabBar->count() == 0) {
        addTab();
        return;
    }

    saveTabs();
}

void TabbingGroup::saveTabs()
{
    config().writeEntry("Tabs", m_tabs);

    emit configNeedsSaving();
}

void TabbingGroup::configModTab()
{
    int pos = m_ui.listWidget->currentRow();

    if (pos == -1) {
        return;
    }

    QString title = m_ui.tileEdit->text();

    if (title.isEmpty()) {
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
    QList<AbstractGroup *> newTabs;
    QStringList tabs;
    for (int i = 0; i < m_ui.listWidget->count(); ++i) {
        QListWidgetItem *item = m_ui.listWidget->item(i);
        tabs << item->text();
        newTabs << m_tabGroups.at(item->data(Qt::UserRole).toInt());
    }

    while (m_tabBar->count() > 0) {
        m_tabBar->takeTab(0);
    }

    for (int i = 0; i < newTabs.count(); ++i) {
        m_tabBar->insertTab(i, tabs.at(i), newTabs.at(i));
    }

    m_tabGroups = newTabs;
    m_tabs = tabs;

    saveChildren();
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
        if (v) {
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
    }

    return false;
}

void TabbingGroup::dragMoveEvent(QGraphicsSceneDragDropEvent *event)
{
    moveToTabAt(static_cast<QGraphicsSceneDragDropEvent *>(event)->scenePos());
}

void TabbingGroup::changeTab()
{
    if (m_changingTab != -1) {
        m_tabBar->setCurrentIndex(m_changingTab);
        m_changingTab = -1;
    }
}

bool TabbingGroup::showDropZone(const QPointF &pos)
{
    moveToTabAt(mapToScene(pos));

    return false;
}

void TabbingGroup::moveToTabAt(const QPointF &scenePos)
{
    QPointF pos = m_actualTabBar->mapFromScene(scenePos);

    int index = m_tabBar->nativeWidget()->tabAt(pos.toPoint());
    if (index == -1) {
        m_changingTab = -1;
        m_changeTabTimer->stop();
    } else {
        m_changingTab = index;
        m_changeTabTimer->start();
    }
}

GroupInfo TabbingGroup::groupInfo()
{
    GroupInfo gi("tabbing", i18n("Tabbing Group"));
    gi.setIcon("tab-new");

    return gi;
}

#include "tabbinggroup.moc"
