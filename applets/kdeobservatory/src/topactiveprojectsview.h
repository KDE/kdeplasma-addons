#ifndef TOPACTIVEPROJECTSVIEW_HEADER
#define TOPACTIVEPROJECTSVIEW_HEADER

#include "iviewprovider.h"

#include "kdeobservatory.h"

class TopActiveProjectsView : public IViewProvider
{
public:
    TopActiveProjectsView(QHash<QString, bool> topActiveProjectsViewProjects, const QMap<QString, KdeObservatory::Project> &projects, const QRectF &rect, QGraphicsItem *parent = 0, Qt::WindowFlags wFlags = 0);
    virtual ~TopActiveProjectsView();

private:
    const QHash<QString, bool> &m_topActiveProjectsViewProjects;
    const QMap<QString, KdeObservatory::Project> &m_projects;
};

#endif
