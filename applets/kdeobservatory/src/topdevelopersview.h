#ifndef TOPDEVELOPERSVIEW_HEADER
#define TOPDEVELOPERSVIEW_HEADER

#include "iviewprovider.h"

#include "kdeobservatory.h"

class TopDevelopersView : public IViewProvider
{
public:
    TopDevelopersView(const QHash<QString, bool> &topDevelopersViewProjects, const QMap<QString, KdeObservatory::Project> &projects, QRectF rect, QGraphicsWidget *parent = 0, Qt::WindowFlags wFlags = 0);
    virtual ~TopDevelopersView();

    virtual void updateViews();

private:
    const QHash<QString, bool> &m_topDevelopersViewProjects;
    const QMap<QString, KdeObservatory::Project> &m_projects;
};

#endif
