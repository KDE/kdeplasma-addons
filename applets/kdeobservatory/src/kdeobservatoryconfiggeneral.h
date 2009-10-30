#ifndef KDEOBSERVATORYCONFIGGENERAL_HEADER
#define KDEOBSERVATORYCONFIGGENERAL_HEADER

#include <QWidget>

namespace Ui
{
    class KdeObservatoryConfigGeneral;
}

class KdeObservatoryConfigGeneral : public QWidget
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

    Ui::KdeObservatoryConfigGeneral *m_configGeneral;
};

#endif
