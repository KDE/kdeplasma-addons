#include "kdeobservatoryview.h"

#include <QResizeEvent>

#include <QDebug>

KdeObservatoryView::KdeObservatoryView(QWidget * parent)
: QGraphicsView(parent)
{
}

KdeObservatoryView::~KdeObservatoryView()
{
}

void KdeObservatoryView::resizeEvent(QResizeEvent *event)
{
    if (scene())
    {
        QSize size = event->size();
        qreal minSizeView = qMin(size.width()-20, size.height()-20);
        QRectF rect = scene()->itemsBoundingRect();
        qreal minSizeScene = (size.width() < size.height()) ? rect.width():rect.height();
        qreal factor = minSizeView/minSizeScene;
        resetMatrix();
        scale(factor, factor);
    }
    QGraphicsView::resizeEvent(event);
}
