#ifndef KDEOBSERVATORYCONFIGPROJECTS_HEADER
#define KDEOBSERVATORYCONFIGPROJECTS_HEADER

#include <QWidget>
#include "ui_kdeobservatoryconfigprojects.h"

class KdeObservatoryConfigProjects : public QWidget, public Ui::KdeObservatoryConfigProjects
{
    Q_OBJECT
public:
    KdeObservatoryConfigProjects(QWidget *parent = 0, Qt::WindowFlags f = 0);
    virtual ~KdeObservatoryConfigProjects();

    void createTableWidgetItem(const QString &projectName, const QString &commitSubject, const QString &icon);

Q_SIGNALS:
    void projectAdded(const QString &projectName, const QString &icon);
    void projectRemoved(const QString &projectName);

public Q_SLOTS:
    void on_psbAddProject_clicked();
    void on_psbRemoveProject_clicked();
    void on_psbEditProject_clicked();
};

#endif
