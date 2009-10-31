#ifndef KDEOBSERVATORYCONFIGGENERAL_HEADER
#define KDEOBSERVATORYCONFIGGENERAL_HEADER

#include <QWidget>
#include "ui_kdeobservatoryconfiggeneral.h"

class KdeObservatoryConfigGeneral : public QWidget, public Ui::KdeObservatoryConfigGeneral
{
    Q_OBJECT
public:
    KdeObservatoryConfigGeneral(QWidget *parent = 0, Qt::WindowFlags f = 0);
    virtual ~KdeObservatoryConfigGeneral();

public Q_SLOTS:
    void on_tlbUp_clicked();
    void on_tlbDown_clicked();

private:
    void swapViewItems(int updown);
};

#endif
