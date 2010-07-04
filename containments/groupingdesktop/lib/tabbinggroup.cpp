
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

TabbingGroup::TabbingGroup(QGraphicsItem* parent, Qt::WindowFlags wFlags) : AbstractGroup(parent, wFlags), current_index(0)
{
    layout = new QGraphicsLinearLayout(Qt::Horizontal);
    setLayout(layout);
    tabbar = new Plasma::TabBar();
    tabwidget = new QGraphicsWidget();
    tabbar->addTab(QString("1"), tabwidget);
    tabbar->addTab(QString("2"), tabwidget);
    layout->addItem(tabbar);

    QGraphicsLinearLayout *sndlayout = new QGraphicsLinearLayout(Qt::Vertical);
    addbutton = new Plasma::PushButton();
    addbutton->setIcon(KIcon("configure"));
    addbutton->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
    sndlayout->addItem(addbutton);
    sndlayout->addItem(new QGraphicsWidget());
    layout->addItem(sndlayout);

    resize(200, 200);
    setGroupType(AbstractGroup::FreeGroup);

    connect(tabbar, SIGNAL(currentChanged(int)),
            this, SLOT(tabbarIndexChanged(int)));

    connect(this, SIGNAL(appletRemovedFromGroup(Plasma::Applet*, AbstractGroup*)),
            this, SLOT(onAppletRemoved(Plasma::Applet*, AbstractGroup*)));
    connect(this, SIGNAL(appletAddedInGroup(Plasma::Applet*, AbstractGroup*)),
            this, SLOT(onAppletAdded(Plasma::Applet*, AbstractGroup*)));
    connect(this, SIGNAL(subGroupRemovedFromGroup(AbstractGroup*, AbstractGroup*)),
            this, SLOT(onSubGroupRemoved(AbstractGroup*, AbstractGroup*)));
    connect(this, SIGNAL(subGroupAddedInGroup(AbstractGroup*, AbstractGroup*)),
            this, SLOT(onSubGroupAdded(AbstractGroup*, AbstractGroup*)));

    connect(addbutton, SIGNAL(clicked()), this, SLOT(callConfig()));
}

TabbingGroup::~TabbingGroup()
{
}

void TabbingGroup::layoutChild(QGraphicsWidget* child, const QPointF& pos)
{
    Q_UNUSED(child)
    Q_UNUSED(pos)
}

QString TabbingGroup::pluginName() const
{
    return QString("tabbing");
}

void TabbingGroup::restoreChildGroupInfo(QGraphicsWidget* child, const KConfigGroup& group)
{ 
    QPointF pos = group.readEntry("Position", QPointF());
    int index = group.readEntry("SiteIndex", 0);
    int key;

    if ((key = m_children.key(child, -1)) != -1)
        m_children.remove(key, child);
    if(index<tabbar->count())
        m_children.insert(index, child);
    else
        m_children.insert(0, child);

    if (index != current_index)
        child->hide();

    child->setPos(pos);
}

void TabbingGroup::saveChildGroupInfo(QGraphicsWidget* child, KConfigGroup group) const
{
    group.writeEntry("Position", child->pos());
    group.writeEntry("SiteIndex", m_children.key(child, 0));
}

void TabbingGroup::onAppletAdded(Plasma::Applet* applet, AbstractGroup*)
{ 
    m_children.insert(tabbar->currentIndex(), applet);
}

void TabbingGroup::onAppletRemoved(Plasma::Applet* applet, AbstractGroup*)
{ 
    m_children.remove(m_children.key(applet), applet);
}

void TabbingGroup::onSubGroupAdded(AbstractGroup* subGroup, AbstractGroup*)
{
    m_children.insert(tabbar->currentIndex(), subGroup);
}

void TabbingGroup::onSubGroupRemoved(AbstractGroup* subGroup, AbstractGroup*)
{
    m_children.remove(m_children.key(subGroup), subGroup);
}

void TabbingGroup::tabbarIndexChanged(int index)
{
    hideChildren(current_index);
    current_index = index;
    showChildren(current_index);
}

void TabbingGroup::hideChildren(int index)
{ 
    QList<QGraphicsWidget*> items = m_children.values(index);
    foreach(QGraphicsWidget* item, items) {
        item->hide();
    }
}

void TabbingGroup::showChildren(int index)
{
    QList<QGraphicsWidget*> items = m_children.values(index);
    foreach(QGraphicsWidget* item, items) {
        item->show();
    }
}


void TabbingGroup::save(KConfigGroup& group) const
{
    AbstractGroup::save(group);
    QString tabs;

    for (int i = 0;i < tabbar->count();++i) {
        if (i > 0)
            tabs += ",";

        tabs += tabbar->tabText(i);
    }

    group.writeEntry("tabs", tabs);

    group.writeEntry("currenttab", current_index);
}

void TabbingGroup::restore(KConfigGroup& group)
{
    QString tabs = group.readEntry("tabs", i18n("Default"));
    renameTabs(tabs.split(","));
    
    //put children from non existing groups in first groups
    QMultiMap<int, QGraphicsWidget*>::iterator it=m_children.begin();
    while(it!=m_children.end())
    {
        if(it.key()>=tabbar->count())
        {
            QGraphicsWidget* item=it.value();
            it=m_children.erase(it);
            m_children.insert(0,item);
        }
        else
            ++it;
    }/*
    hideChildren(current_index);
    current_index = group.readEntry("currenttab", 0);
    showChildren(current_index);*/
    tabbar->setCurrentIndex(group.readEntry("currenttab", 0));
    AbstractGroup::restore(group); 
}


void TabbingGroup::createConfigurationInterface(KConfigDialog *parent)
{
    QWidget *widget = new QWidget();
    ui.setupUi(widget);
    config_mapper.clear();

    for (int i = 0;i < tabbar->count();++i) {
        config_mapper.append(i);
        ui.listWidget->addItem(tabbar->tabText(i));
    }

    connect(ui.addButton, SIGNAL(clicked()), this, SLOT(configAddTab()));

    connect(ui.modButton, SIGNAL(clicked()), this, SLOT(configModTab()));
    connect(ui.delButton, SIGNAL(clicked()), this, SLOT(configDelTab()));
    connect(ui.upButton, SIGNAL(clicked()), this, SLOT(configUpTab()));
    connect(ui.downButton, SIGNAL(clicked()), this, SLOT(configDownTab()));

    parent->addPage(widget, i18n("General"), "configure");
    connect(parent, SIGNAL(applyClicked()), this, SLOT(configAccepted()));
    connect(parent, SIGNAL(okClicked()), this, SLOT(configAccepted()));
}

void TabbingGroup::configAddTab()
{
    int pos = ui.listWidget->currentRow();

    if (pos == -1) //insert after current position in list
        pos = ui.listWidget->count();
    else
        ++pos;

    config_mapper.insert(pos, -1);

    QString title = ui.tileEdit->text();

    if (title == "")
        title = "Default";

    ui.listWidget->insertItem(pos, title);

    ui.listWidget->setCurrentRow(pos);
}

void TabbingGroup::configDelTab()
{
    if (ui.listWidget->count() == 1)
        return;

    int pos = ui.listWidget->currentRow();

    if (pos == -1)
        return;

    config_mapper.removeAt(pos);

    ui.listWidget->takeItem(pos);
}

void TabbingGroup::configModTab()
{
    int pos = ui.listWidget->currentRow();

    if (pos == -1)
        return;

    QString title = ui.tileEdit->text();

    if (title == "")
        return;

    ui.listWidget->item(pos)->setText(title);
}

void TabbingGroup::configUpTab()
{
    int pos = ui.listWidget->currentRow();

    if (pos < 1)
        return;

    config_mapper.swap(pos, pos - 1);

    QListWidgetItem *item = ui.listWidget->takeItem(pos);

    ui.listWidget->insertItem(pos - 1, item);

    ui.listWidget->setCurrentRow(pos - 1);
}

void TabbingGroup::configDownTab()
{
    int pos = ui.listWidget->currentRow();

    if (pos == -1 || pos == ui.listWidget->count() - 1)
        return;

    config_mapper.swap(pos, pos + 1);

    QListWidgetItem *item = ui.listWidget->takeItem(pos);

    ui.listWidget->insertItem(pos + 1, item);

    ui.listWidget->setCurrentRow(pos + 1);
}

void TabbingGroup::configAccepted()
{ 
    hideChildren(current_index);
    
    int future_index = 0;
    QMultiMap<int, QGraphicsWidget*> new_m;
    for (int i = 0;i < config_mapper.count();++i) {
        if (config_mapper[i] != -1) {
            QMultiMap<int, QGraphicsWidget*>::iterator it = m_children.find(config_mapper[i]);

            while (it != m_children.end() && it.key() == config_mapper[i]) {
                new_m.insert(i, it.value());
                it = m_children.erase(it);
            }

            if (config_mapper[i] == current_index) {
                future_index = i;
            }
        }
    }

    QMultiMap<int, QGraphicsWidget*>::iterator it = m_children.begin();
    while (it != m_children.end()) {
        new_m.insert(0, it.value());
        it = m_children.erase(it);
    }


    QStringList titles;
    for (int i = 0; i < ui.listWidget->count();++i) {
        titles.append(ui.listWidget->item(i)->text());
    }
    renameTabs(titles);

    current_index = future_index;
    m_children = new_m;
    tabbar->setCurrentIndex(current_index);
    showChildren(current_index);
}

void TabbingGroup::renameTabs(QStringList titles)
{ 
    qDebug() << titles;
    int current_count = tabbar->count();

    if (current_count > titles.count()) {
        for (int i = titles.count();i < current_count;++i) {
            tabbar->removeTab(0);
        }

        current_count = titles.count();
    }

    int index = 0;

    foreach(QString tab, titles) {
        if (index < current_count)
            tabbar->setTabText(index, tab);
        else
            tabbar->addTab(tab, tabwidget);

        ++index;
    }
}

void TabbingGroup::callConfig()
{
    
    if (KConfigDialog::showDialog("tabbingconfig"))
        return;
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
