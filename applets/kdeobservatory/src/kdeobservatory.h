#ifndef KDEOBSERVATORY_HEADER
#define KDEOBSERVATORY_HEADER

#include <Plasma/Applet>

class QGraphicsScene;
class QGraphicsView;

class KdeObservatory : public Plasma::Applet
{
    Q_OBJECT
public:
    KdeObservatory(QObject *parent, const QVariantList &args);
    ~KdeObservatory();

    void init();

private slots:
    void createConfigurationInterface(KConfigDialog *parent);

private:
    QGraphicsScene *m_scene;
    QGraphicsView  *m_view;
};

K_EXPORT_PLASMA_APPLET(kdeobservatory, KdeObservatory)

#endif
