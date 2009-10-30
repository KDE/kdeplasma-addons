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

private Q_SLOTS:
    void on_psbAddProject_clicked();

private:
    Ui::KdeObservatoryConfigGeneral *m_configGeneral;
};

#endif
