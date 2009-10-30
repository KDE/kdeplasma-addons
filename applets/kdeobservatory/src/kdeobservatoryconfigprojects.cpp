#include "kdeobservatoryconfigprojects.h"

#include <KDialog>

#include "ui_kdeobservatoryconfigprojects.h"
#include "ui_kdeobservatoryconfigproject.h"

KdeObservatoryConfigProjects::KdeObservatoryConfigProjects(QWidget *parent, Qt::WindowFlags f)
: QWidget(parent, f), m_configProjects(new Ui::KdeObservatoryConfigProjects)
{
    m_configProjects->setupUi(this);
    m_configProjects->projects->horizontalHeader()->setStretchLastSection(true);
    m_configProjects->projects->setRowCount(0);
}

KdeObservatoryConfigProjects::~KdeObservatoryConfigProjects()
{
    delete m_configProjects;
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
        QTableWidgetItem *itemCommitSubject = new QTableWidgetItem(ui_configProject->commitSubject->text());
        int rowCount = m_configProjects->projects->rowCount();
        m_configProjects->projects->setRowCount(rowCount+1);
        m_configProjects->projects->setItem(rowCount, 0, itemProject);
        m_configProjects->projects->setItem(rowCount, 1, itemCommitSubject);
        m_configProjects->projects->setRowHeight(rowCount, m_configProjects->projects->rowHeight(rowCount)*0.75);
    }
    delete ui_configProject;
    delete configProject;
}

void KdeObservatoryConfigProjects::on_psbRemoveProject_clicked()
{
}

void KdeObservatoryConfigProjects::on_psbEditProject_clicked()
{
}
