#include "kdeobservatoryconfiggeneral.h"

#include <KDialog>

#include "ui_kdeobservatoryconfiggeneral.h"
#include "ui_kdeobservatoryconfigproject.h"

KdeObservatoryConfigGeneral::KdeObservatoryConfigGeneral(QWidget *parent, Qt::WindowFlags f)
: QWidget(parent, f), m_configGeneral(new Ui::KdeObservatoryConfigGeneral)
{
    m_configGeneral->setupUi(this);
}

KdeObservatoryConfigGeneral::~KdeObservatoryConfigGeneral()
{
    delete m_configGeneral;
}

void KdeObservatoryConfigGeneral::on_psbAddProject_clicked()
{
    QPointer<KDialog> configProject = new KDialog(this);
    configProject->setButtons(KDialog::None);
    Ui::KdeObservatoryConfigProject *ui_configProject = new Ui::KdeObservatoryConfigProject;
    ui_configProject->setupUi(configProject);

    if (configProject->exec() == KDialog::Accepted)
    {
    }

    delete ui_configProject;
    delete configProject;
}
