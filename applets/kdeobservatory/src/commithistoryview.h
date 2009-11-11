#ifndef COMMITHISTORYVIEW_HEADER
#define COMMITHISTORYVIEW_HEADER

#include "iviewprovider.h"

#include "kdeobservatory.h"

class CommitHistoryView : public IViewProvider
{
public:
    CommitHistoryView(const QHash<QString, bool> &commitHistoryViewProjects, const QMap<QString, KdeObservatory::Project> &projects, QRectF rect, QGraphicsWidget *parent = 0, Qt::WindowFlags wFlags = 0);
    virtual ~CommitHistoryView();

    virtual void updateViews();

private:
    const QHash<QString, bool> &m_commitHistoryViewProjects;
    const QMap<QString, KdeObservatory::Project> &m_projects;
};

#endif
