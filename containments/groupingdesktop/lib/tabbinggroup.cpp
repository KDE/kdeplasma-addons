
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

#include <KDE/KConfigDialog>

#include <Plasma/TabBar>
#include <Plasma/PushButton>

TabbingGroup::TabbingGroup(QGraphicsItem *parent, Qt::WindowFlags wFlags)
            : AbstractGroup(parent, wFlags),
              m_tabbar(new Plasma::TabBar(this)),
              m_layout(new QGraphicsLinearLayout(Qt::Horizontal)),
              m_current_index(0),
              m_tabwidget(new QGraphicsWidget(this)),
              m_addbutton(new Plasma::PushButton(this))
{
    setLayout(m_layout);

    m_tabbar->addTab(QString("1"), m_tabwidget);
    m_tabbar->addTab(QString("2"), m_tabwidget);
    m_layout->addItem(m_tabbar);

    QGraphicsLinearLayout *sndlayout = new QGraphicsLinearLayout(Qt::Vertical);
    m_addbutton->setIcon(KIcon("configure"));
    m_addbutton->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
    sndlayout->addItem(m_addbutton);
    sndlayout->addItem(new QGraphicsWidget());
    m_layout->addItem(sndlayout);

    resize(200, 200);
    setGroupType(AbstractGroup::FreeGroup);

    connect(m_tabbar, SIGNAL(currentChanged(int)),
            this, SLOT(tabbarIndexChanged(int)));

    connect(this, SIGNAL(appletRemovedFromGroup(Plasma::Applet*, AbstractGroup*)),
            this, SLOT(onAppletRemoved(Plasma::Applet*, AbstractGroup*)));
    connect(this, SIGNAL(appletAddedInGroup(Plasma::Applet*, AbstractGroup*)),
            this, SLOT(onAppletAdded(Plasma::Applet*, AbstractGroup*)));
    connect(this, SIGNAL(subGroupRemovedFromGroup(AbstractGroup*, AbstractGroup*)),
            this, SLOT(onSubGroupRemoved(AbstractGroup*, AbstractGroup*)));
    connect(this, SIGNAL(subGroupAddedInGroup(AbstractGroup*, AbstractGroup*)),
            this, SLOT(onSubGroupAdded(AbstractGroup*, AbstractGroup*)));

    connect(m_addbutton, SIGNAL(clicked()), this, SLOT(callConfig()));
}

TabbingGroup::~TabbingGroup()
{
}

void TabbingGroup::layoutChild(QGraphicsWidget *child, const QPointF &pos)
{
    Q_UNUSED(child)
    Q_UNUSED(pos)
}

QString TabbingGroup::pluginName() const
{
    return QString("tabbing");
}

void TabbingGroup::restoreChildGroupInfo(QGraphicsWidget *child, const KConfigGroup &group)
{
    QPointF pos = group.readEntry("Position", QPointF());
    int index = group.readEntry("SiteIndex", 0);
    int key;

    if ((key = m_children.key(child, -1)) != -1) {
        m_children.remove(key, child);
    }
    if (index < m_tabbar->count()) {
        m_children.insert(index, child);
    } else {
        m_children.insert(0, child);
    }

    if (index != m_current_index) {
        child->hide();
    }

    child->setPos(pos);
}

void TabbingGroup::saveChildGroupInfo(QGraphicsWidget *child, KConfigGroup group) const
{
    group.writeEntry("Position", child->pos());
    group.writeEntry("SiteIndex", m_children.key(child, 0));
}

void TabbingGroup::onAppletAdded(Plasma::Applet *applet, AbstractGroup *)
{
    m_children.insert(m_tabbar->currentIndex(), applet);
}

void TabbingGroup::onAppletRemoved(Plasma::Applet *applet, AbstractGroup *)
{
    m_children.remove(m_children.key(applet), applet);
}

void TabbingGroup::onSubGroupAdded(AbstractGroup *subGroup, AbstractGroup *)
{
    m_children.insert(m_tabbar->currentIndex(), subGroup);
}

void TabbingGroup::onSubGroupRemoved(AbstractGroup *subGroup, AbstractGroup *)
{
    m_children.remove(m_children.key(subGroup), subGroup);
}

void TabbingGroup::tabbarIndexChanged(int index)
{
    hideChildren(m_current_index);
    m_current_index = index;
    showChildren(m_current_index);
}

void TabbingGroup::hideChildren(int index)
{
    QList<QGraphicsWidget *> items = m_children.values(index);
    foreach(QGraphicsWidget *item, items) {
        item->hide();
    }
}

void TabbingGroup::showChildren(int index)
{
    QList<QGraphicsWidget *> items = m_children.values(index);
    foreach(QGraphicsWidget * item, items) {
        item->show();
    }
}


void TabbingGroup::save(KConfigGroup &group) const
{
    AbstractGroup::save(group);
    QString tabs;

    for (int i = 0;i < m_tabbar->count();++i) {
        if (i > 0) {
            tabs += ',';
        }

        tabs += m_tabbar->tabText(i);
    }

    group.writeEntry("tabs", tabs);
    group.writeEntry("currenttab", m_current_index);
}

void TabbingGroup::restore(KConfigGroup &group)
{
    AbstractGroup::restore(group);

    QString tabs = group.readEntry("tabs", i18n("Default"));
    renameTabs(tabs.split(","));

    //put children from non existing groups in first groups
    QMultiMap<int, QGraphicsWidget *>::iterator it = m_children.begin();
    while (it != m_children.end()) {
        if (it.key() >= m_tabbar->count()) {
            QGraphicsWidget *item = it.value();
            it = m_children.erase(it);
            m_children.insert(0, item);
        } else {
            ++it;
        }
    }/*
    hideChildren(current_index);
    current_index = group.readEntry("currenttab", 0);
    showChildren(current_index);*/
    m_tabbar->setCurrentIndex(group.readEntry("currenttab", 0));
}


void TabbingGroup::createConfigurationInterface(KConfigDialog *parent)
{
    QWidget *widget = new QWidget();
    m_ui.setupUi(widget);
    m_config_mapper.clear();

    for (int i = 0;i < m_tabbar->count();++i) {
        m_config_mapper.append(i);
        m_ui.listWidget->addItem(m_tabbar->tabText(i));
    }

    connect(m_ui.addButton, SIGNAL(clicked()), this, SLOT(configAddTab()));

    connect(m_ui.modButton, SIGNAL(clicked()), this, SLOT(configModTab()));
    connect(m_ui.delButton, SIGNAL(clicked()), this, SLOT(configDelTab()));
    connect(m_ui.upButton, SIGNAL(clicked()), this, SLOT(configUpTab()));
    connect(m_ui.downButton, SIGNAL(clicked()), this, SLOT(configDownTab()));

    parent->addPage(widget, i18n("General"), "configure");
    connect(parent, SIGNAL(applyClicked()), this, SLOT(configAccepted()));
    connect(parent, SIGNAL(okClicked()), this, SLOT(configAccepted()));
}

void TabbingGroup::configAddTab()
{
    int pos = m_ui.listWidget->currentRow();

    if (pos == -1) { //insert after current position in list
        pos = m_ui.listWidget->count();
    } else {
        ++pos;
    }

    m_config_mapper.insert(pos, -1);

    QString title = m_ui.tileEdit->text();

    if (title == QString()) {
        title = "Default";
    }

    m_ui.listWidget->insertItem(pos, title);

    m_ui.listWidget->setCurrentRow(pos);
}

void TabbingGroup::configDelTab()
{
    if (m_ui.listWidget->count() == 1) {
        return;
    }

    int pos = m_ui.listWidget->currentRow();

    if (pos == -1) {
        return;
    }

    m_config_mapper.removeAt(pos);

    m_ui.listWidget->takeItem(pos);
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

    m_config_mapper.swap(pos, pos - 1);

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

    m_config_mapper.swap(pos, pos + 1);

    QListWidgetItem *item = m_ui.listWidget->takeItem(pos);

    m_ui.listWidget->insertItem(pos + 1, item);
    m_ui.listWidget->setCurrentRow(pos + 1);
}

void TabbingGroup::configAccepted()
{
    hideChildren(m_current_index);

    int future_index = 0;
    QMultiMap<int, QGraphicsWidget *> new_m;
    for (int i = 0; i < m_config_mapper.count(); ++i) {
        if (m_config_mapper[i] != -1) {
            QMultiMap<int, QGraphicsWidget *>::iterator it = m_children.find(m_config_mapper[i]);

            while (it != m_children.end() && it.key() == m_config_mapper[i]) {
                new_m.insert(i, it.value());
                it = m_children.erase(it);
            }

            if (m_config_mapper[i] == m_current_index) {
                future_index = i;
            }
        }
    }

    QMultiMap<int, QGraphicsWidget *>::iterator it = m_children.begin();
    while (it != m_children.end()) {
        new_m.insert(0, it.value());
        it = m_children.erase(it);
    }


    QStringList titles;
    for (int i = 0; i < m_ui.listWidget->count();++i) {
        titles.append(m_ui.listWidget->item(i)->text());
    }
    renameTabs(titles);

    m_current_index = future_index;
    m_children = new_m;
    m_tabbar->setCurrentIndex(m_current_index);
    showChildren(m_current_index);
}

void TabbingGroup::renameTabs(const QStringList &titles)
{
    qDebug() << titles;
    int current_count = m_tabbar->count();

    if (current_count > titles.count()) {
        for (int i = titles.count();i < current_count;++i) {
            m_tabbar->removeTab(0);
        }

        current_count = titles.count();
    }

    int index = 0;

    foreach(const QString &tab, titles) {
        if (index < current_count) {
            m_tabbar->setTabText(index, tab);
        } else {
            m_tabbar->addTab(tab, m_tabwidget);
        }

        ++index;
    }
}

void TabbingGroup::callConfig()
{
    if (KConfigDialog::showDialog("tabbingconfig")) {
        return;
    }

    KConfigSkeleton *nullManager = new KConfigSkeleton(0);
    KConfigDialog *dialog = new KConfigDialog(0, "tabbingconfig", nullManager);
    dialog->setFaceType(KPageDialog::Auto);
    dialog->setWindowTitle(i18n("Tabbinggroup Configuration"));
    dialog->setAttribute(Qt::WA_DeleteOnClose, true);
    createConfigurationInterface(dialog);
    //TODO: Apply button does not correctly work for now, so do not show it
    dialog->showButton(KDialog::Apply, false);
    dialog->showButton(KDialog::Default, false);
    //QObject::connect(dialog, SIGNAL(applyClicked()), q, SLOT(configDialogFinished()));
    //QObject::connect(dialog, SIGNAL(okClicked()), q, SLOT(configDialogFinished()));
    QObject::connect(dialog, SIGNAL(finished()), nullManager, SLOT(deleteLater()));

    dialog->show();
}

#include "tabbinggroup.moc"
