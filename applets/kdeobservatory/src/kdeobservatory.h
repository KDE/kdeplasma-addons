#ifndef KDEOBSERVATORY_HEADER
#define KDEOBSERVATORY_HEADER

#include <KConfigGroup>

#include <Plasma/Applet>

class QTimer;
class QTimeLine;
class QGraphicsProxyWidget;
class QGraphicsLinearLayout;

class CommitCollector;
class TopDevelopersView;
class TopActiveProjectsView;
class KdeObservatoryConfigGeneral;
class KdeObservatoryConfigProjects;
class KdeObservatoryConfigTopDevelopers;
class KdeObservatoryConfigTopActiveProjects;

namespace Plasma
{
    class Label;
    class PushButton;
}

namespace Ui
{
    class KdeObservatoryConfigTopActiveProjects;
}

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
    void prepareUpdateViews();
    void updateViews();

    KConfigGroup m_configGroup;

    KdeObservatoryConfigGeneral *m_configGeneral;
    KdeObservatoryConfigProjects *m_configProjects;
    KdeObservatoryConfigTopActiveProjects *m_configTopActiveProjects;
    KdeObservatoryConfigTopDevelopers *m_configTopDevelopers;

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

    // Config - Top Active Projects
    QHash<QString, bool> m_topActiveProjectsViewProjects;

    // Config - Top Developers
    QHash<QString, bool> m_topDevelopersViewProjects;

    // Main Layout
    QGraphicsLinearLayout *m_horizontalLayout;
    QGraphicsWidget *m_viewContainer;
    QGraphicsProxyWidget *m_progressProxy;
    Plasma::Label *m_updateLabel;
    Plasma::PushButton *m_right;
    Plasma::PushButton *m_left;

    // View providers
    TopActiveProjectsView *m_topActiveProjectsView;
    TopDevelopersView     *m_topDevelopersView;

    QList<QGraphicsWidget *> m_views;
    int m_currentView;

    QTimer *m_viewTransitionTimer;
    QTimer *m_synchronizationTimer;
    QTimeLine *m_transitionTimer;

    CommitCollector *m_collector;
};

#endif
