#ifndef KRAZYREPORTVIEW_HEADER
#define KRAZYREPORTVIEW_HEADER

#include "iviewprovider.h"

#include "kdeobservatory.h"

class KrazyReportView : public IViewProvider
{
public:
    KrazyReportView(const QHash<QString, bool> &krazyReportViewProjects, const QMap<QString, KdeObservatory::Project> &projects, QRectF rect, QGraphicsWidget *parent = 0, Qt::WindowFlags wFlags = 0);
    virtual ~KrazyReportView();

    virtual void updateViews();

private:
    const QHash<QString, bool> &m_krazyReportViewProjects;
    const QMap<QString, KdeObservatory::Project> &m_projects;
};

#endif
