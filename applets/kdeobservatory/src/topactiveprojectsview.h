#ifndef TOPACTIVEPROJECTSVIEW_HEADER
#define TOPACTIVEPROJECTSVIEW_HEADER

#include "iviewprovider.h"

#include "kdeobservatory.h"

class ICollector;

class TopActiveProjectsView : public IViewProvider
{
public:
    TopActiveProjectsView(const QMap<QString, KdeObservatory::Project> &projectsInView, ICollector *collector, const QRectF &rect, QGraphicsItem *parent = 0, Qt::WindowFlags wFlags = 0);
    virtual ~TopActiveProjectsView();

private:
    const QMap<QString, KdeObservatory::Project> &m_projectsInView;
    ICollector *m_collector;
};

#endif
