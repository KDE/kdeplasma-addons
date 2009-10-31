#include "kdeobservatoryconfiggeneral.h"

KdeObservatoryConfigGeneral::KdeObservatoryConfigGeneral(QWidget *parent, Qt::WindowFlags f)
: QWidget(parent, f)
{
    setupUi(this);
}

KdeObservatoryConfigGeneral::~KdeObservatoryConfigGeneral()
{
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
    int linenumber = activeViews->currentRow();

    if (linenumber + updown < activeViews->count())
    {
        QListWidgetItem *item = activeViews->currentItem();
        activeViews->takeItem(linenumber);
        activeViews->insertItem(linenumber + updown, item);
        activeViews->setCurrentItem(item);
    }
}
