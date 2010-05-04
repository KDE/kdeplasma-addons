/*************************************************************************
 * Copyright 2009-2010 Sandro Andrade sandroandrade@kde.org              *
 *                                                                       *
 * This program is free software; you can redistribute it and/or         *
 * modify it under the terms of the GNU General Public License as        *
 * published by the Free Software Foundation; either version 2 of        *
 * the License or (at your option) version 3 or any later version        *
 * accepted by the membership of KDE e.V. (or its successor approved     *
 * by the membership of KDE e.V.), which shall act as a proxy            *
 * defined in Section 14 of version 3 of the license.                    *
 *                                                                       *
 * This program is distributed in the hope that it will be useful,       *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 * GNU General Public License for more details.                          *
 *                                                                       *
 * You should have received a copy of the GNU General Public License     *
 * along with this program.  If not, see <http://www.gnu.org/licenses/>. *
 * ***********************************************************************/

#include "kdeobservatoryconfigviews.h"

KdeObservatoryConfigViews::KdeObservatoryConfigViews(QWidget *parent, Qt::WindowFlags f)
: QWidget(parent, f)
{
    setupUi(this);
    psbCheckAll->setIcon(KIcon("button_more"));
    psbUncheckAll->setIcon(KIcon("button_fewer"));
    m_lastView = views->currentText();
}

KdeObservatoryConfigViews::~KdeObservatoryConfigViews()
{
}

void KdeObservatoryConfigViews::projectAdded(const QString &projectName, const QString &icon)
{
    KdeObservatory::Project project;
    project.icon = icon;
    m_projects[projectName] = project;
    QMapIterator<QString, ViewProjects> i(m_projectsInView);
    while (i.hasNext())
    {
        i.next();
        QString view = i.key();
        m_projectsInView[view][projectName] = true;
    }
    on_views_currentIndexChanged(m_lastView);
}

void KdeObservatoryConfigViews::projectRemoved(const QString &projectName)
{
    m_projects.remove(projectName);
    QMapIterator<QString, ViewProjects> i(m_projectsInView);
    while (i.hasNext())
    {
        i.next();
        QString view = i.key();
        m_projectsInView[view].remove(projectName);
    }
    updateView(m_lastView);
}

void KdeObservatoryConfigViews::on_views_currentIndexChanged(QString view)
{
    int counter = projectsInView->count();
    for (int i = 0; i < counter; ++i)
    {
        QListWidgetItem *item = projectsInView->item(i);
        m_projectsInView[m_lastView][item->text()] = item->checkState() == Qt::Checked ? true:false;
    }
    updateView(view);
}

void KdeObservatoryConfigViews::updateView(const QString &view)
{
    projectsInView->clear();
    QHashIterator<QString, bool> i(m_projectsInView[view]);
    while (i.hasNext())
    {
        i.next();
        QString project = i.key();
        QListWidgetItem *item = new QListWidgetItem(project, projectsInView);
        item->setCheckState(i.value() ? Qt::Checked:Qt::Unchecked);
        item->setIcon(KIcon(m_projects[project].icon));
    }
    m_lastView = view;
}

void KdeObservatoryConfigViews::on_tlbUp_clicked()
{
    swapViewItems(-1);
}

void KdeObservatoryConfigViews::on_tlbDown_clicked()
{
    swapViewItems(1);
}

void KdeObservatoryConfigViews::on_psbCheckAll_clicked()
{
    int counter = projectsInView->count();
    for (int i = 0; i < counter; ++i)
        projectsInView->item(i)->setCheckState(Qt::Checked);
}

void KdeObservatoryConfigViews::on_psbUncheckAll_clicked()
{
    int counter = projectsInView->count();
    for (int i = 0; i < counter; ++i)
        projectsInView->item(i)->setCheckState(Qt::Unchecked);
}

void KdeObservatoryConfigViews::swapViewItems(int updown)
{
    int linenumber = activeViews->currentRow();

    if (linenumber + updown < activeViews->count())
    {
        QListWidgetItem *item = activeViews->currentItem();
        activeViews->takeItem(linenumber);
        activeViews->insertItem(linenumber + updown, item);
        activeViews->setCurrentItem(item);
    }
}
