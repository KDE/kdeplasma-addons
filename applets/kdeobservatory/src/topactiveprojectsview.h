#ifndef TOPACTIVEPROJECTSVIEW_HEADER
#define TOPACTIVEPROJECTSVIEW_HEADER

#include "iview.h"

#include "kdeobservatory.h"

class ICollector;

class TopActiveProjectsView : public IView
{
public:
    TopActiveProjectsView(const QMap<QString, KdeObservatory::Project> &projects, ICollector *collector, const QString &title, const QRectF &rect, QGraphicsItem *parent = 0, Qt::WindowFlags wFlags = 0);
    virtual ~TopActiveProjectsView();

private:
    const QMap<QString, KdeObservatory::Project> &m_projects;
    ICollector *m_collector;
};

#endif
