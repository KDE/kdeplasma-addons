#ifndef ICOLLECTOR_HEADER
#define ICOLLECTOR_HEADER

#include <QHttp>

class ICollector : public QHttp
{
    Q_OBJECT
public:
    ICollector(QObject *parent = 0);
    virtual ~ICollector();

    virtual void run() = 0;

Q_SIGNALS:
    void progressMaximum(int value);
    void progressValue(int value);
    void collectFinished();
    void collectError(QString error);

protected Q_SLOTS:
    virtual void requestFinished (int id, bool error) = 0;
};

#endif
