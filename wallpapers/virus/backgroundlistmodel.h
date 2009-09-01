/*
  Copyright (c) 2007 Paolo Capriotti <p.capriotti@gmail.com>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.
*/

#ifndef BACKGROUNDLISTMODEL_H
#define BACKGROUNDLISTMODEL_H

#include <QAbstractListModel>
#include <QPixmap>

#include <KDirWatch>
#include <KFileItem>

#include <Plasma/Wallpaper>

class QEventLoop;
class KProgressDialog;

namespace Plasma
{
    class Package;
} // namespace Plasma

class BackgroundListModel : public QAbstractListModel
{
    Q_OBJECT

public:
    BackgroundListModel(float ratio, Plasma::Wallpaper *listener, QObject *parent);
    virtual ~BackgroundListModel();

    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    Plasma::Package *package(int index) const;

    void reload();
    void reload(const QStringList &selected);
    void addBackground(const QString &path);
    QModelIndex indexOf(const QString &path) const;
    virtual bool contains(const QString &bg) const;

    static QStringList findAllBackgrounds(Plasma::Wallpaper *structureParent,
                                                       const BackgroundListModel *container,
                                                       const QStringList &path);
    static void initProgressDialog(KProgressDialog *dialog);

    void setWallpaperSize(QSize size);
    void setResizeMethod(Plasma::Wallpaper::ResizeMethod resizeMethod);

protected Q_SLOTS:
    void removeBackground(const QString &path);
    void showPreview(const KFileItem &item, const QPixmap &preview);
    void previewFailed(const KFileItem &item);

private:
    QSize bestSize(Plasma::Package *package) const;

    Plasma::Wallpaper *m_listener;
    Plasma::Wallpaper *m_structureParent;
    QList<Plasma::Package *> m_packages;
    QHash<Plasma::Package *, QSize> m_sizeCache;
    QHash<Plasma::Package *, QPixmap> m_previews;
    QHash<KUrl, QPersistentModelIndex> m_previewJobs;
    float m_ratio;
    KDirWatch m_dirwatch;

    QSize m_size;
    Plasma::Wallpaper::ResizeMethod m_resizeMethod;
};

class BackgroundFinder : public QObject
{
    Q_OBJECT

public:
    BackgroundFinder(Plasma::Wallpaper *structureParent,
                     const BackgroundListModel *container,
                     const QStringList &p,
                     QEventLoop *eventLoop);

    QStringList papersFound() const;

public slots:
    void start();

signals:
    void finished();

private:
    Plasma::Wallpaper *m_structureParent;
    const BackgroundListModel *m_container;
    QStringList m_paths;
    QStringList m_papersFound;
    QEventLoop *m_eventLoop;
};

#endif // BACKGROUNDLISTMODEL_H
