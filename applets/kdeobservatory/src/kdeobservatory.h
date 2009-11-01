#ifndef KDEOBSERVATORY_HEADER
#define KDEOBSERVATORY_HEADER

#include <QStandardItemModel>

#include <KConfigGroup>

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

protected Q_SLOTS:
    void createConfigurationInterface(KConfigDialog *parent);
    void configAccepted();

private:
    KConfigGroup m_configGroup;

    QGraphicsScene *m_scene;
    QGraphicsView  *m_view;

    KdeObservatoryConfigGeneral *m_configGeneral;
    KdeObservatoryConfigProjects *m_configProjects;

    QStandardItemModel *m_projects;
};

#endif
