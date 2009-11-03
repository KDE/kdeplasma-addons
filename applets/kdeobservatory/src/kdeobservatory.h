#ifndef KDEOBSERVATORY_HEADER
#define KDEOBSERVATORY_HEADER

#include <KConfigGroup>

#include <Plasma/Applet>

class QGraphicsView;
class QGraphicsScene;
class QStandardItemModel;

class KdeObservatoryConfigGeneral;
class KdeObservatoryConfigProjects;
class CommitCollector;

class KdeObservatory : public Plasma::Applet
{
    Q_OBJECT
public:
    KdeObservatory(QObject *parent, const QVariantList &args);
    ~KdeObservatory();

    void init();

    struct Project
    {
        QString name;
        QString commitSubject;
        QString icon;
    };

protected Q_SLOTS:
    void createConfigurationInterface(KConfigDialog *parent);
    void configAccepted();
    void collectFinished();

private:
    KConfigGroup m_configGroup;

    QGraphicsScene *m_scene;
    QGraphicsView  *m_view;

    KdeObservatoryConfigGeneral *m_configGeneral;
    KdeObservatoryConfigProjects *m_configProjects;

    int m_commitExtent;
    int m_synchronizationDelay;
    bool m_cacheContents;
    bool m_enableAnimations;
    bool m_enableTransitionEffects;
    bool m_enableAutoViewChange;
    int m_viewsDelay;

    QList<Project> m_projects;
    QList< QPair<QString, bool> > m_views;

    CommitCollector *m_collector;
};

#endif
