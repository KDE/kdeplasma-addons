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

#include "kdeobservatoryconfigprojects.h"

#include <QStandardItem>

#include <KDialog>
#include <KMessageBox>

#include "ui_kdeobservatoryconfigproject.h"

KdeObservatoryConfigProjects::KdeObservatoryConfigProjects(QWidget *parent, Qt::WindowFlags f)
: QWidget(parent, f)
{
    setupUi(this);
    psbAddProject->setIcon(KIcon("list-add"));
    psbRemoveProject->setIcon(KIcon("list-remove"));
    psbEditProject->setIcon(KIcon("document-edit"));
}

KdeObservatoryConfigProjects::~KdeObservatoryConfigProjects()
{
}

void KdeObservatoryConfigProjects::createTableWidgetItem(const QString &projectName, const QString &commitSubject, const QString &krazyReport, const QString &krazyFilePrefix, const QString &icon)
{
    QTableWidgetItem *itemProject = new QTableWidgetItem(KIcon(icon), projectName);
    itemProject->setData(Qt::UserRole, icon);
    QTableWidgetItem *itemCommitSubject = new QTableWidgetItem(commitSubject);
    QTableWidgetItem *itemKrazyReport = new QTableWidgetItem(krazyReport);
    QTableWidgetItem *itemKrazyFilePrefix = new QTableWidgetItem(krazyFilePrefix);
    int rowCount = projects->rowCount();
    projects->setRowCount(rowCount+1);
    projects->setItem(rowCount, 0, itemProject);
    projects->setItem(rowCount, 1, itemCommitSubject);
    projects->setItem(rowCount, 2, itemKrazyReport);
    projects->setItem(rowCount, 3, itemKrazyFilePrefix);
    projects->setRowHeight(rowCount, projects->rowHeight(rowCount)*0.75);
    projects->setCurrentItem(itemProject);
}

void KdeObservatoryConfigProjects::on_psbAddProject_clicked()
{
    QPointer<KDialog> configProject = new KDialog(this);
    configProject->setButtons(KDialog::None);
    Ui::KdeObservatoryConfigProject *ui_configProject = new Ui::KdeObservatoryConfigProject;
    ui_configProject->setupUi(configProject);

    if (configProject->exec() == KDialog::Accepted)
    {
        createTableWidgetItem(ui_configProject->projectName->text(), ui_configProject->commitSubject->text(), ui_configProject->krazyReport->text(), ui_configProject->krazyFilePrefix->text(), ui_configProject->icon->icon());
        projects->resizeColumnsToContents();
        projects->horizontalHeader()->setStretchLastSection(true);
        emit projectAdded(ui_configProject->projectName->text(), ui_configProject->icon->icon());
    }

    delete ui_configProject;
    delete configProject;
}

void KdeObservatoryConfigProjects::on_psbRemoveProject_clicked()
{
    QTableWidgetItem *currentItem;
    if ((currentItem = projects->currentItem()))
        if (KMessageBox::questionYesNo(this, i18n("Are you sure you want to remove project '%1'?", projects->item(currentItem->row(), 0)->text() ), i18n("Remove project")) == KMessageBox::Yes)
        {
            QString projectName = currentItem->text();
            projects->removeRow(currentItem->row());
            emit projectRemoved(projectName);
        }
}

void KdeObservatoryConfigProjects::on_psbEditProject_clicked()
{
    QTableWidgetItem *currentItem;
    if ((currentItem = projects->currentItem()))
    {
        QPointer<KDialog> configProjects = new KDialog(this);
        configProjects->setButtons(KDialog::None);
        Ui::KdeObservatoryConfigProject *ui_configProjects = new Ui::KdeObservatoryConfigProject;
        ui_configProjects->setupUi(configProjects);

        int currentRow = projects->currentRow();
        ui_configProjects->projectName->setText(projects->item(currentRow, 0)->text());
        ui_configProjects->commitSubject->setText(projects->item(currentRow, 1)->text());
        ui_configProjects->krazyReport->setText(projects->item(currentRow, 2)->text());
        ui_configProjects->krazyFilePrefix->setText(projects->item(currentRow, 3)->text());
        ui_configProjects->icon->setIcon(projects->item(currentRow, 0)->data(Qt::UserRole).toString());

        if (configProjects->exec() == KDialog::Accepted)
        {
            projects->item(currentRow, 0)->setText(ui_configProjects->projectName->text());
            projects->item(currentRow, 0)->setIcon(KIcon(ui_configProjects->icon->icon()));
            projects->item(currentRow, 0)->setData(Qt::UserRole, ui_configProjects->icon->icon());
            projects->item(currentRow, 1)->setText(ui_configProjects->commitSubject->text());
            projects->item(currentRow, 2)->setText(ui_configProjects->krazyReport->text());
            projects->item(currentRow, 3)->setText(ui_configProjects->krazyFilePrefix->text());
	    emit projectEdited(projects->item(currentRow, 0)->text());
        }

        delete ui_configProjects;
        delete configProjects;
    }
}
