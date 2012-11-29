#ifndef BACKGROUNDLISTMODEL_CPP
#define BACKGROUNDLISTMODEL_CPP
/*
  Copyright (c) 2007 Paolo Capriotti <p.capriotti@gmail.com>
  Copyright 2011 Reza Fatahilah Shah <rshah0385@kireihana.com>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.
*/

#include "backgroundlistmodel.h"

#include <QFile>
#include <QDir>
#include <QThreadPool>
#include <QUuid>
#include <QPainter>

#include <KDebug>
#include <KFileMetaInfo>
#include <KGlobal>
#include <KIO/PreviewJob>
#include <KProgressDialog>
#include <KStandardDirs>

#include <Plasma/Package>
#include <Plasma/PackageStructure>

#include "backgrounddelegate.h"
#include "pattern.h"

#define PATTERN_RESOURCE_TYPE "dtop_pattern"
#define PATTERN_CONFIG_GROUP "KDE Desktop Pattern"
#define COMMENT_CONFIG_KEY "Comment"
#define FILE_CONFIG_KEY "File"
#define COMMENT_CONFIG_KEY "Comment"

BackgroundListModel::BackgroundListModel(PatternWallpaper *listener, QObject *parent)
    : QAbstractListModel(parent),
      m_structureParent(listener)
{
    connect(&m_dirwatch, SIGNAL(deleted(QString)), this, SLOT(removeBackground(QString)));
    m_previewUnavailablePix.fill(Qt::transparent);
}

BackgroundListModel::~BackgroundListModel()
{
    qDeleteAll(m_kconfigs);
}

void BackgroundListModel::removeBackground(const QString &path)
{
    QModelIndex index;
    while ((index = indexOf(path)).isValid()) {
        beginRemoveRows(QModelIndex(), index.row(), index.row());
        KConfig *config = m_kconfigs.at(index.row());
        m_kconfigs.removeAt(index.row());
        delete config;
        endRemoveRows();
    }
}

void BackgroundListModel::reload()
{
    reload(QStringList());
}

void BackgroundListModel::reload(const QStringList &selected)
{
    if (!m_kconfigs.isEmpty()) {
        beginRemoveRows(QModelIndex(), 0, m_kconfigs.count() - 1);
        qDeleteAll(m_kconfigs);
        m_kconfigs.clear();
        endRemoveRows();
    }

    if (!selected.isEmpty()) {
        processPaths(selected);
    }

    const QStringList dirs = KGlobal::dirs()->findAllResources(PATTERN_RESOURCE_TYPE, QLatin1String("*.desktop"), KStandardDirs::NoDuplicates);

    kDebug() << "going looking in" << dirs;
    processPaths(dirs);
}

void BackgroundListModel::processPaths(const QStringList &paths)
{
    QList<KConfig *> newKConfigs;
    foreach (const QString &file, paths) {
        if (!contains(file) && QFile::exists(file)) {
            newKConfigs << new KConfig(file);
        }
    }
    // add new files to dirwatch
    foreach (KConfig *config, newKConfigs) {
        if (!m_dirwatch.contains(config->name())) {
            m_dirwatch.addFile(config->name());
        }
    }

    if (!newKConfigs.isEmpty()) {
        const int start = rowCount();
        beginInsertRows(QModelIndex(), start, start + newKConfigs.size());
        m_kconfigs.append(newKConfigs);
        endInsertRows();
    }

    //kDebug() << t.elapsed();
}

QModelIndex BackgroundListModel::indexOf(const QString &path) const
{
    //kDebug() << "indexOf=" << path;
    for (int i = 0; i < m_kconfigs.size(); i++) {
        if (path == m_kconfigs[i]->name()) {
            //kDebug() << "return PATH" << path;
            return index(i, 0);
        } else {
            //Check if the supplied parameter is filename
            KConfigGroup patternConfig(m_kconfigs[i], PATTERN_CONFIG_GROUP);
            QString package = patternConfig.readEntry(FILE_CONFIG_KEY, QString());
            kDebug() << "Config name=" << package;
            if (path == package) {
                return index(i, 0);
            }
        }
    }
    return QModelIndex();
}

bool BackgroundListModel::contains(const QString &path) const
{
    return indexOf(path).isValid();
}

int BackgroundListModel::rowCount(const QModelIndex &) const
{
    return m_kconfigs.size();
}

QVariant BackgroundListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    if (index.row() >= m_kconfigs.size()) {
        return QVariant();
    }

    KConfig *config = kconfig(index.row());
    if (!config) {
        return QVariant();
    }

    KConfigGroup patternConfig(config, PATTERN_CONFIG_GROUP);

    switch (role) {
    case Qt::DisplayRole: {
        QString title = patternConfig.readEntry(COMMENT_CONFIG_KEY, QString());
        if (title.isEmpty()) {
            return QFileInfo(config->name()).completeBaseName();
        }

        return title;
    }
    break;

    case BackgroundDelegate::ScreenshotRole: {
        if (m_previews.contains(config)) {
            return m_previews.value(config);
        }

        KUrl file(KGlobal::dirs()->findResource(PATTERN_RESOURCE_TYPE, 
                                                patternConfig.readEntry(FILE_CONFIG_KEY, QString())));

        if (!m_previewJobs.contains(file) && file.isValid()) {
            KFileItemList list;
            list.append(KFileItem(file, QString(), 0));
            KIO::PreviewJob* job = KIO::filePreview(list,
                                                    QSize(BackgroundDelegate::SCREENSHOT_SIZE,
                                                    BackgroundDelegate::SCREENSHOT_SIZE/1.6));
            job->setIgnoreMaximumSize(true);
            connect(job, SIGNAL(gotPreview(KFileItem,QPixmap)),
                    this, SLOT(showPreview(KFileItem,QPixmap)));
            connect(job, SIGNAL(failed(KFileItem)),
                    this, SLOT(previewFailed(KFileItem)));
            const_cast<BackgroundListModel *>(this)->m_previewJobs.insert(file, QPersistentModelIndex(index));
        }

        const_cast<BackgroundListModel *>(this)->m_previews.insert(config, m_previewUnavailablePix);
        return m_previewUnavailablePix;
    }
    break;

    default:
        return QVariant();
    break;
    }
    return QVariant();
}

void BackgroundListModel::showPreview(const KFileItem &item, const QPixmap &preview)
{
    QPersistentModelIndex index = m_previewJobs.value(item.url());
    m_previewJobs.remove(item.url());

    if (!index.isValid()) {
        return;
    }

    KConfig *config = kconfig(index.row());
    if (!config) {
        return;
    }

    if (m_structureParent.isNull()) {
        return;
    }

    QPixmap pix(m_size);
    QPainter p(&pix);
    QImage image = preview.toImage();
    p.drawTiledPixmap(pix.rect(), m_structureParent.data()->generatePattern(image), QPoint(0,0));
    p.end();

    m_previews.insert(config, pix);
    //kDebug() << "preview size:" << preview.size();
    m_structureParent.data()->updateScreenshot(index);
}

void BackgroundListModel::previewFailed(const KFileItem &item)
{
    m_previewJobs.remove(item.url());
}

KConfig* BackgroundListModel::kconfig(int index) const
{
    return m_kconfigs.at(index);
}

void BackgroundListModel::setWallpaperSize(const QSize& size)
{
    float newHeight = ((float)size.height() / (float)size.width()) * BackgroundDelegate::SCREENSHOT_SIZE;

    m_size = QSize(BackgroundDelegate::SCREENSHOT_SIZE, newHeight);

    m_size.scale(BackgroundDelegate::SCREENSHOT_SIZE, BackgroundDelegate::SCREENSHOT_SIZE/1.6, Qt::KeepAspectRatio);
}
#include "backgroundlistmodel.moc"


#endif // BACKGROUNDLISTMODEL_CPP
