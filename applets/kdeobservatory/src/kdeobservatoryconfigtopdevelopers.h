#ifndef KDEOBSERVATORYCONFIGTOPDEVELOPERS_HEADER
#define KDEOBSERVATORYCONFIGTOPDEVELOPERS_HEADER

#include <QWidget>
#include "ui_kdeobservatoryconfigtopdevelopers.h"

class KdeObservatoryConfigTopDevelopers : public QWidget, public Ui::KdeObservatoryConfigTopDevelopers
{
    Q_OBJECT
public:
    KdeObservatoryConfigTopDevelopers(QWidget *parent = 0, Qt::WindowFlags f = 0);
    virtual ~KdeObservatoryConfigTopDevelopers();

    void createListWidgetItem(const QString &projectName, const QString &icon, bool active);

public Q_SLOTS:
    void projectAdded(const QString &projectName, const QString &icon);
    void projectRemoved(const QString &projectName);
};

#endif
