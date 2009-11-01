#include "kdeobservatoryconfigprojects.h"

#include <QStandardItem>

#include <KDialog>
#include <KMessageBox>

#include "ui_kdeobservatoryconfigproject.h"

#include <QDebug>

KdeObservatoryConfigProjects::KdeObservatoryConfigProjects(QWidget *parent, Qt::WindowFlags f)
: QWidget(parent, f)
{
    setupUi(this);
    projects->verticalHeader()->hide();
}

KdeObservatoryConfigProjects::~KdeObservatoryConfigProjects()
{
}

void KdeObservatoryConfigProjects::on_psbAddProject_clicked()
{
    QPointer<KDialog> configProject = new KDialog(this);
    configProject->setButtons(KDialog::None);
    Ui::KdeObservatoryConfigProject *ui_configProject = new Ui::KdeObservatoryConfigProject;
    ui_configProject->setupUi(configProject);

    if (configProject->exec() == KDialog::Accepted)
    {
        QTableWidgetItem *itemProject = new QTableWidgetItem(KIcon(ui_configProject->icon->icon()), ui_configProject->projectName->text());
        itemProject->setData(Qt::UserRole, ui_configProject->icon->icon());
        QTableWidgetItem *itemCommitSubject = new QTableWidgetItem(ui_configProject->commitSubject->text());
        int rowCount = projects->rowCount();
        projects->setRowCount(rowCount+1);
        projects->setItem(rowCount, 0, itemProject);
        projects->setItem(rowCount, 1, itemCommitSubject);
        projects->setRowHeight(rowCount, projects->rowHeight(rowCount)*0.75);
        projects->setCurrentItem(itemProject);
        projects->resizeColumnsToContents();
        projects->horizontalHeader()->setStretchLastSection(true);
    }

    delete ui_configProject;
    delete configProject;
}

void KdeObservatoryConfigProjects::on_psbRemoveProject_clicked()
{
    QTableWidgetItem *currentItem;
    if ((currentItem = projects->currentItem()))
        if (KMessageBox::questionYesNo(this, "Are you sure you want to remove project '" + currentItem->text() + "' ?", "Remove projet") == KMessageBox::Yes)
            projects->removeRow(currentItem->row());
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
        ui_configProjects->icon->setIcon(projects->item(currentRow, 0)->icon());

        if (configProjects->exec() == KDialog::Accepted)
        {
            projects->item(currentRow, 0)->setText(ui_configProjects->projectName->text());
            projects->item(currentRow, 0)->setIcon(KIcon(ui_configProjects->icon->icon()));
            projects->item(currentRow, 0)->setData(Qt::UserRole, ui_configProjects->icon->icon());
            projects->item(currentRow, 1)->setText(ui_configProjects->commitSubject->text());
        }

        delete ui_configProjects;
        delete configProjects;
    }
}
