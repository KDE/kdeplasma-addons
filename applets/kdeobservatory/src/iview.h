#ifndef IVIEW_HEADER
#define IVIEW_HEADER

#include <QGraphicsWidget>

class QGraphicsWidget;

namespace Plasma
{
    class Frame;
}

class IView : public QObject
{
public:
    IView(const QRectF &rect, QGraphicsItem *parent = 0, Qt::WindowFlags wFlags = 0);
    virtual ~IView();

    QGraphicsWidget *createView(const QString &title);
    const QList<QGraphicsWidget *> views() const;

protected:
    const QRectF &m_rect;
    QGraphicsItem *m_parent;
    Qt::WindowFlags m_wFlags;
    QList<QGraphicsWidget *> m_views;
};

#endif
