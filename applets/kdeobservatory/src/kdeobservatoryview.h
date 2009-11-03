#ifndef KDEOBSERVATORYVIEW_HEADER
#define KDEOBSERVATORYVIEW_HEADER

#include <QGraphicsView>

class QResizeEvent;

class KdeObservatoryView : public QGraphicsView
{
    Q_OBJECT
public:
    KdeObservatoryView(QWidget * parent = 0);
    ~KdeObservatoryView();

protected:
    void resizeEvent(QResizeEvent *event);
};

#endif
