#ifndef KDEOBSERVATORY_HEADER
#define KDEOBSERVATORY_HEADER

#include <KConfigGroup>

#include <Plasma/Applet>

class QTimer;
class QProgressBar;
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
        QString commitSubject;
        QString icon;
    };

protected Q_SLOTS:
    void createConfigurationInterface(KConfigDialog *parent);
    void configAccepted();
    void collectFinished();
    void moveViewRight();
    void moveViewLeft();
    void switchViews(int delta);
    void runCollectors();

private:
    KConfigGroup m_configGroup;

    KdeObservatoryConfigGeneral *m_configGeneral;
    KdeObservatoryConfigProjects *m_configProjects;

    // Config - General
    int  m_commitExtent;
    int  m_synchronizationDelay;
    bool m_cacheContents;
    bool m_enableAnimations;
    bool m_enableTransitionEffects;
    bool m_enableAutoViewChange;
    int  m_viewsDelay;
    QList< QPair<QString, bool> > m_activeViews;

    // Config - Projects
    QMap<QString, Project> m_projects;

    // Main Layout
    QGraphicsWidget *m_viewContainer;
    QProgressBar *m_collectorProgress;

    QList<QGraphicsWidget *> m_views;
    int m_currentView;

    QTimer *m_viewTransitionTimer;
    QTimer *m_synchronizationTimer;

    CommitCollector *m_collector;
};

#endif
