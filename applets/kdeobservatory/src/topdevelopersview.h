#ifndef TOPDEVELOPERSVIEW_HEADER
#define TOPDEVELOPERSVIEW_HEADER

#include "iviewprovider.h"

#include "kdeobservatory.h"

class TopDevelopersView : public IViewProvider
{
public:
    TopDevelopersView(QHash<QString, bool> topDevelopersViewProjects, const QMap<QString, KdeObservatory::Project> &projects, const QRectF &rect, QGraphicsItem *parent = 0, Qt::WindowFlags wFlags = 0);
    virtual ~TopDevelopersView();

private:
    const QHash<QString, bool> &m_topDevelopersViewProjects;
    const QMap<QString, KdeObservatory::Project> &m_projects;
};

#endif
