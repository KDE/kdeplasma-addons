#include "kdeobservatory.h"

#include <QGraphicsProxyWidget>
#include <QGraphicsLinearLayout>
#include <QGraphicsScene>
#include <QGraphicsView>

#include <KConfigDialog>

#include "kdeobservatoryconfiggeneral.h"
#include "kdeobservatoryconfigprojects.h"
#include "ui_kdeobservatoryconfigcommitsummary.h"

#include "commitcollector.h"

KdeObservatory::KdeObservatory(QObject *parent, const QVariantList &args)
: Plasma::Applet(parent, args)
{
    setBackgroundHints(DefaultBackground);
    setHasConfigurationInterface(true);  
    resize(200, 200);

    QGraphicsProxyWidget *proxy = new QGraphicsProxyWidget(this);
    proxy->setWidget(m_view = new QGraphicsView);
    m_view->setScene(m_scene = new QGraphicsScene);
    //m_view->setBackgroundBrush(QColor(0, 0, 0));
    //m_view->setAutoFillBackground(true);
    m_scene->addEllipse(0, 0, 50, 50);

    QGraphicsLinearLayout *layout = new QGraphicsLinearLayout;
    layout->addItem(proxy);
    setLayout(layout);

    CommitCollector *c = new CommitCollector(this);
    //c->run();
}

KdeObservatory::~KdeObservatory()
{
}

void KdeObservatory::init()
{
}

void KdeObservatory::createConfigurationInterface(KConfigDialog *parent)
{
    KdeObservatoryConfigGeneral *configGeneral = new KdeObservatoryConfigGeneral;
    parent->addPage(configGeneral, i18n("General"), "applications-development");

    KdeObservatoryConfigProjects *configProjects = new KdeObservatoryConfigProjects;
    parent->addPage(configProjects, i18n("Projects"), "project-development");

    QWidget *configCommitSummary = new QWidget;
    Ui::KdeObservatoryConfigCommitSummary *ui_configCommitSummary = new Ui::KdeObservatoryConfigCommitSummary;
    ui_configCommitSummary->setupUi(configCommitSummary);
    parent->addPage(configCommitSummary, i18n("Commit Summary"), "svn-commit");
}

#include "kdeobservatory.moc"

