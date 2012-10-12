/*
  Copyright (c) 2007 Paolo Capriotti <p.capriotti@gmail.com>
  Copyright 2011 Reza Fatahilah Shah <rshah0385@kireihana.com>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.
*/

#ifndef BACKGROUNDLISTMODEL_H
#define BACKGROUNDLISTMODEL_H

#include <QAbstractListModel>
#include <QPixmap>
#include <QRunnable>
#include <QThread>

#include <KDirWatch>
#include <KFileItem>

#include <Plasma/Wallpaper>

class QEventLoop;
class KProgressDialog;

namespace Plasma
{
    class Package;
} // namespace Plasma

class PatternWallpaper;

class BackgroundListModel : public QAbstractListModel
{
    Q_OBJECT

public:
    BackgroundListModel(PatternWallpaper *listener, QObject *parent);
    virtual ~BackgroundListModel();

    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    KConfig *kconfig(int index) const;

    void reload();
    void reload(const QStringList &selected);
    QModelIndex indexOf(const QString &path) const;
    virtual bool contains(const QString &bg) const;

    void setWallpaperSize(const QSize& size);
    void setResizeMethod(Plasma::Wallpaper::ResizeMethod resizeMethod);

protected Q_SLOTS:
    void removeBackground(const QString &path);
    void showPreview(const KFileItem &item, const QPixmap &preview);
    void previewFailed(const KFileItem &item);
    void processPaths(const QStringList &paths);

private:
    QWeakPointer<PatternWallpaper> m_structureParent;

    QList<KConfig *> m_kconfigs;
    QHash<KConfig *, QPixmap> m_previews;


    QHash<KUrl, QPersistentModelIndex> m_previewJobs;
    KDirWatch m_dirwatch;

    QString m_findToken;
    QPixmap m_previewUnavailablePix;
    QSize m_size;
};

#endif // BACKGROUNDLISTMODEL_H
