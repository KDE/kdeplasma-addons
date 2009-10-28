#ifndef ICOLLECTOR_HEADER
#define ICOLLECTOR_HEADER

#include <QHttp>

class ICollector : public QHttp
{
    Q_OBJECT
public:
    ICollector(QObject *parent = 0);
    virtual ~ICollector();

    typedef QMap<QString, QList< QPair<QString, int> > > ResultMap;
    virtual void run() = 0;

    const QString &summary() const;
    const ResultMap &resultMap() const;

Q_SIGNALS:
    void collectFinished();
    void collectError(QString error);

protected Q_SLOTS:
    virtual void requestFinished (int id, bool error) = 0;

protected:
    QString m_summary;
    ResultMap m_resultMap;
};

#endif
