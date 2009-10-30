#include "kdeobservatoryconfiggeneral.h"

#include "ui_kdeobservatoryconfiggeneral.h"

KdeObservatoryConfigGeneral::KdeObservatoryConfigGeneral(QWidget *parent, Qt::WindowFlags f)
: QWidget(parent, f), m_configGeneral(new Ui::KdeObservatoryConfigGeneral)
{
    m_configGeneral->setupUi(this);
}

KdeObservatoryConfigGeneral::~KdeObservatoryConfigGeneral()
{
    delete m_configGeneral;
}

void KdeObservatoryConfigGeneral::on_tlbUp_clicked()
{
    swapViewItems(-1);
}

void KdeObservatoryConfigGeneral::on_tlbDown_clicked()
{
    swapViewItems(1);
}

void KdeObservatoryConfigGeneral::swapViewItems(int updown)
{
    int linenumber = m_configGeneral->activeViews->currentRow();

    if (linenumber + updown < m_configGeneral->activeViews->count())
    {
        QListWidgetItem *item = m_configGeneral->activeViews->currentItem();
        m_configGeneral->activeViews->takeItem(linenumber);
        m_configGeneral->activeViews->insertItem(linenumber + updown, item);
        m_configGeneral->activeViews->setCurrentItem(item);
    }
}
