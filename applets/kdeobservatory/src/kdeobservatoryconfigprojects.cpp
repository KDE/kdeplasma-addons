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
    projects->horizontalHeader()->setStretchLastSection(true);
    projects->verticalHeader()->hide();
    projects->resizeColumnsToContents();
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
        QStandardItem *nameItem = new QStandardItem(KIcon(ui_configProject->icon->icon()), ui_configProject->projectName->text());
        nameItem->setData(ui_configProject->icon->icon(), Qt::UserRole);
        QStandardItem *commitSubjectItem = new QStandardItem(ui_configProject->commitSubject->text());
        (qobject_cast<QStandardItemModel *>(projects->model()))->appendRow(QList<QStandardItem *>() << nameItem << commitSubjectItem);
        projects->setCurrentIndex(nameItem->index());
    }

    delete ui_configProject;
    delete configProject;
}

void KdeObservatoryConfigProjects::on_psbRemoveProject_clicked()
{
    QStandardItemModel *model = qobject_cast<QStandardItemModel *>(projects->model());
    QModelIndex currentIndex;
    if ((currentIndex = projects->currentIndex()).isValid())
        if (KMessageBox::questionYesNo(this, "Are you sure you want to remove project '" + model->data(currentIndex, Qt::DisplayRole).toString() + "' ?", "Remove projet") == KMessageBox::Yes)
            model->removeRow(currentIndex.row());
}

void KdeObservatoryConfigProjects::on_psbEditProject_clicked()
{
    QModelIndex currentIndex;
    if ((currentIndex = projects->currentIndex()).isValid())
    {
        QPointer<KDialog> configProjects = new KDialog(this);
        configProjects->setButtons(KDialog::None);
        Ui::KdeObservatoryConfigProject *ui_configProjects = new Ui::KdeObservatoryConfigProject;
        ui_configProjects->setupUi(configProjects);

        int currentRow = currentIndex.row();
        QStandardItemModel *model = qobject_cast<QStandardItemModel *>(projects->model());
        QModelIndex index0 = model->index(currentRow, 0, QModelIndex());
        QModelIndex index1 = model->index(currentRow, 1, QModelIndex());
        ui_configProjects->projectName->setText(model->data(index0, Qt::DisplayRole).toString());
        ui_configProjects->commitSubject->setText(model->data(index1, Qt::DisplayRole).toString());
        ui_configProjects->icon->setIcon(model->data(index0, Qt::UserRole).toString());

        if (configProjects->exec() == KDialog::Accepted)
        {
            model->setData(index0, ui_configProjects->projectName->text(), Qt::DisplayRole);
            model->setData(index1, ui_configProjects->commitSubject->text(), Qt::DisplayRole);
            model->itemFromIndex(index0)->setIcon(KIcon(ui_configProjects->icon->icon()));
            model->setData(index0, ui_configProjects->icon->icon(), Qt::UserRole);
        }

        delete ui_configProjects;
        delete configProjects;
    }
}
