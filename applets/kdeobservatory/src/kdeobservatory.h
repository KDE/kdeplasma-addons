#ifndef KDEOBSERVATORY_HEADER
#define KDEOBSERVATORY_HEADER

#include <Plasma/Applet>

class QGraphicsScene;
class QGraphicsView;

class KDEObservatory : public Plasma::Applet
{
    Q_OBJECT
public:
    KDEObservatory(QObject *parent, const QVariantList &args);
    ~KDEObservatory();

    void init();

private slots:
    void createConfigurationInterface(KConfigDialog *parent);

private:
    QGraphicsScene *m_scene;
    QGraphicsView  *m_view;
};

K_EXPORT_PLASMA_APPLET(kdeobservatory, KDEObservatory)

#endif
