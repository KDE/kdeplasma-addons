#ifndef _KDED_POTD_H_
#define _KDED_POTD_H_

#include <QObject>
#include <QString>
#include <QFileSystemWatcher>

#include <KConfig>
#include <KConfigGroup>
#include <KDEDModule>
#include <Plasma/DataEngine>
#include <Plasma/DataEngineConsumer>

class PotdModule: public KDEDModule
{
    Q_OBJECT

public:
    PotdModule(QObject* parent, const QList<QVariant>&);
    virtual ~PotdModule();

private Q_SLOTS:
    void fileChanged(const QString &path);

private:
    QString getSource();

    Plasma::DataEngineConsumer *consumer;
    Plasma::DataEngine *engine;
    QFileSystemWatcher *watcher;
    QString configPath;
    QString previousSource;
};

#endif
