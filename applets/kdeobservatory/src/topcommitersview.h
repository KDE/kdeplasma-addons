#ifndef TOPCOMMITERSVIEW_HEADER
#define TOPCOMMITERSVIEW_HEADER

#include "iviewprovider.h"

class ICollector;

class TopCommitersView : public IViewProvider
{
public:
    TopCommitersView(ICollector *collector, const QRectF &rect, QGraphicsItem *parent = 0, Qt::WindowFlags wFlags = 0);
    virtual ~TopCommitersView();

private:
    ICollector *m_collector;
};

#endif
