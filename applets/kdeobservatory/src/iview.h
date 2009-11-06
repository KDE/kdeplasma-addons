#ifndef IVIEW_HEADER
#define IVIEW_HEADER

#include <QGraphicsWidget>

class QGraphicsWidget;

namespace Plasma
{
    class Frame;
}

class IView : public QGraphicsWidget
{
public:
    IView(const QString &title, const QRectF &rect, QGraphicsItem *parent = 0, Qt::WindowFlags wFlags = 0);
    virtual ~IView();

protected:
    Plasma::Frame *m_header;
    QGraphicsWidget *m_container;
};

#endif
