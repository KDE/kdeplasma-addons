#ifndef KDEOBSERVATORYCONFIGTOPACTIVEPROJECTS_HEADER
#define KDEOBSERVATORYCONFIGTOPACTIVEPROJECTS_HEADER

#include <QWidget>
#include "ui_kdeobservatoryconfigtopactiveprojects.h"

class KdeObservatoryConfigTopActiveProjects : public QWidget, public Ui::KdeObservatoryConfigTopActiveProjects
{
    Q_OBJECT
public:
    KdeObservatoryConfigTopActiveProjects(QWidget *parent = 0, Qt::WindowFlags f = 0);
    virtual ~KdeObservatoryConfigTopActiveProjects();

    void createListWidgetItem(const QString &projectName, const QString &icon, bool active);

public Q_SLOTS:
    void projectAdded(const QString &projectName, const QString &icon);
    void projectRemoved(const QString &projectName);
};

#endif
