#ifndef TOPACTIVEPROJECTSVIEW_HEADER
#define TOPACTIVEPROJECTSVIEW_HEADER

#include <QGraphicsRectItem>

#include "kdeobservatory.h"

class ICollector;

class TopActiveProjectsView : public QGraphicsRectItem
{
public:
    TopActiveProjectsView(const QMap<QString, KdeObservatory::Project> &projects, ICollector *collector, const QRectF &rect, QGraphicsItem *parent = 0);
    virtual ~TopActiveProjectsView();

private:
    const QMap<QString, KdeObservatory::Project> &m_projects;
    ICollector *m_collector;
};

#endif
