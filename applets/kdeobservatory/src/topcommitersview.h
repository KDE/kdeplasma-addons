#ifndef TOPCOMMITERSVIEW_HEADER
#define TOPCOMMITERSVIEW_HEADER

#include "iview.h"

#include "kdeobservatory.h"

class ICollector;

class TopCommitersView : public IView
{
public:
    TopCommitersView(const QMap<QString, KdeObservatory::Project> &projects, ICollector *collector, const QRectF &rect, QGraphicsItem *parent = 0, Qt::WindowFlags wFlags = 0);
    virtual ~TopCommitersView();

private:
    const QMap<QString, KdeObservatory::Project> &m_projects;
    ICollector *m_collector;
};

#endif
