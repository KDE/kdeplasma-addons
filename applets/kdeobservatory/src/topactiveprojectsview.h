#ifndef TOPACTIVEPROJECTSVIEW_HEADER
#define TOPACTIVEPROJECTSVIEW_HEADER

#include "iviewprovider.h"

#include "kdeobservatory.h"

class TopActiveProjectsView : public IViewProvider
{
public:
    TopActiveProjectsView(const QHash<QString, bool> &topActiveProjectsViewProjects, const QMap<QString, KdeObservatory::Project> &projects, QRectF rect, QGraphicsWidget *parent = 0, Qt::WindowFlags wFlags = 0);
    virtual ~TopActiveProjectsView();

    virtual void updateViews();

private:
    const QHash<QString, bool> &m_topActiveProjectsViewProjects;
    const QMap<QString, KdeObservatory::Project> &m_projects;
};

#endif
