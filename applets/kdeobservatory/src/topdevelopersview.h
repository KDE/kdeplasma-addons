#ifndef TOPDEVELOPERSVIEW_HEADER
#define TOPDEVELOPERSVIEW_HEADER

#include "iviewprovider.h"

#include "kdeobservatory.h"

class TopDevelopersView : public IViewProvider
{
public:
    TopDevelopersView(const QMap<QString, KdeObservatory::Project> &projectsInView, const QRectF &rect, QGraphicsItem *parent = 0, Qt::WindowFlags wFlags = 0);
    virtual ~TopDevelopersView();

private:
    const QMap<QString, KdeObservatory::Project> &m_projectsInView;
};

#endif
