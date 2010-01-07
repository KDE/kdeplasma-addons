//krazy:skip

namespace Plasma
{

class PLASMA_EXPORT ScrollBar : public QGraphicsProxyWidget
{
    Q_OBJECT

    Q_PROPERTY(int singleStep READ singleStep WRITE setSingleStep)
    Q_PROPERTY(int pageStep READ pageStep WRITE setPageStep)
    Q_PROPERTY(int value READ value WRITE setValue NOTIFY valueChanged)
    Q_PROPERTY(int minimum READ minimum)
    Q_PROPERTY(int maximum READ maximum)
    Q_PROPERTY(QString stylesheet READ styleSheet WRITE setStyleSheet)
};

}
