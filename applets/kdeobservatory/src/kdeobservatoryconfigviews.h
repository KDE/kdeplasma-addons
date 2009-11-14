#ifndef KDEOBSERVATORYCONFIGVIEWS_HEADER
#define KDEOBSERVATORYCONFIGVIEWS_HEADER

#include <QWidget>

#include "ui_kdeobservatoryconfigviews.h"
#include "kdeobservatory.h"

class KdeObservatoryConfigViews : public QWidget, public Ui::KdeObservatoryConfigViews
{
    Q_OBJECT
public:
    KdeObservatoryConfigViews(QWidget *parent = 0, Qt::WindowFlags f = 0);
    virtual ~KdeObservatoryConfigViews();

    typedef QHash<QString, bool> ViewProjects;
    QMap<QString, KdeObservatory::Project> m_projects;
    QMap<QString, ViewProjects> m_projectsInView;

public Q_SLOTS:
    void projectAdded(const QString &projectName, const QString &icon);
    void projectRemoved(const QString &projectName);
    void on_views_currentIndexChanged(QString view);
    void updateView(const QString &view);

private:
    QString m_lastView;
};

#endif
