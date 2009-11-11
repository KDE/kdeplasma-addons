#ifndef IVIEWPROVIDER_HEADER
#define IVIEWPROVIDER_HEADER

#include <QGraphicsWidget>

class QGraphicsWidget;

namespace Plasma
{
    class Frame;
}

class IViewProvider : public QObject
{
public:
    IViewProvider(QRectF rect, QGraphicsWidget *parent = 0, Qt::WindowFlags wFlags = 0);
    virtual ~IViewProvider();

    QGraphicsWidget *createView(const QString &title);
    void deleteViews();
    const QList<QGraphicsWidget *> views() const;

    virtual void updateViews() = 0;

protected:
    QRectF m_rect;
    QGraphicsWidget *m_parent;
    Qt::WindowFlags m_wFlags;
    QList<QGraphicsWidget *> m_views;
};

#endif
