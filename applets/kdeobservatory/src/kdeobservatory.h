#ifndef KDEOBSERVATORY_HEADER
#define KDEOBSERVATORY_HEADER

#include <QDataStream>

#include <Plasma/Applet>

class QGraphicsScene;
class QGraphicsView;

class KdeObservatoryConfigGeneral;
class KdeObservatoryConfigProjects;

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

private:
    QGraphicsScene *m_scene;
    QGraphicsView  *m_view;

    KdeObservatoryConfigGeneral *m_configGeneral;
    KdeObservatoryConfigProjects *m_configProjects;

    QList<Project> m_projects;
};

K_EXPORT_PLASMA_APPLET(kdeobservatory, KdeObservatory)

#endif
