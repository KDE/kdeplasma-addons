/*
  Copyright (c) 2007 Paolo Capriotti <p.capriotti@gmail.com>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.
*/

#include "backgroundlistmodel.h"

#include <QCoreApplication>
#include <QFile>
#include <QDir>
#include <QProgressBar>

#include <KDebug>
#include <KFileMetaInfo>
#include <KGlobal>
#include <KIO/PreviewJob>
#include <KProgressDialog>
#include <KStandardDirs>

#include <Plasma/Package>
#include <Plasma/PackageStructure>

#include "backgrounddelegate.h"
#include "weatherwallpaper.h"

BackgroundListModel::BackgroundListModel(float ratio, Plasma::Wallpaper *listener, QObject *parent)
    : QAbstractListModel(parent),
      m_listener(listener),
      m_structureParent(listener),
      m_ratio(ratio),
      m_size(0,0),
      m_resizeMethod(Plasma::Wallpaper::ScaledResize)
{
    connect(&m_dirwatch, SIGNAL(deleted(QString)), this, SLOT(removeBackground(QString)));
}

BackgroundListModel::~BackgroundListModel()
{
    qDeleteAll(m_packages);
}

void BackgroundListModel::removeBackground(const QString &path)
{
    int index;
    while ((index = indexOf(path)) != -1) {
        beginRemoveRows(QModelIndex(), index, index);
        Plasma::Package *package = m_packages.at(index);
        m_packages.removeAt(index);
        delete package;
        endRemoveRows();
    }
}

void BackgroundListModel::reload()
{
    reload(QStringList());
}

void BackgroundListModel::reload(const QStringList &selected)
{
    QStringList dirs = KGlobal::dirs()->findDirs("wallpaper", "");
    QList<Plasma::Package *> tmp;

    if (!m_packages.isEmpty()) {
        beginRemoveRows(QModelIndex(), 0, m_packages.count() - 1);
        qDeleteAll(m_packages);
        m_packages.clear();
        endRemoveRows();
    }

    foreach (const QString &file, selected) {
        if (!contains(file) && QFile::exists(file)) {
            tmp << new Plasma::Package(file, Plasma::Wallpaper::packageStructure(m_structureParent));
        }
    }

    {
        KProgressDialog progressDialog;
        initProgressDialog(&progressDialog);

        foreach (const QString &dir, dirs) {
            tmp += findAllBackgrounds(m_structureParent, this, dir, m_ratio, &progressDialog);
        }
    }

    // add new files to dirwatch
    foreach (Plasma::Package *b, tmp) {
        //TODO: packages need to be added to the dir watch as well
        if (!m_dirwatch.contains(b->path())) {
            m_dirwatch.addFile(b->path());
        }
    }

    if (!tmp.isEmpty()) {
        beginInsertRows(QModelIndex(), 0, tmp.size() - 1);
        m_packages = tmp;
        endInsertRows();
    }
}

void BackgroundListModel::addBackground(const QString& path)
{
    if (!contains(path)) {
        if (!m_dirwatch.contains(path)) {
            m_dirwatch.addFile(path);
        }
        beginInsertRows(QModelIndex(), 0, 0);
        Plasma::PackageStructure::Ptr structure = Plasma::Wallpaper::packageStructure(m_structureParent);
        Plasma::Package *pkg = new Plasma::Package(path, structure);
        m_packages.prepend(pkg);
        endInsertRows();
    }
}

int BackgroundListModel::indexOf(const QString &path) const
{
    for (int i = 0; i < m_packages.size(); i++) {
        if (path.startsWith(m_packages[i]->path())) {
            return i;
        }
    }
    return -1;
}

bool BackgroundListModel::contains(const QString &path) const
{
    return indexOf(path) != -1;
}

int BackgroundListModel::rowCount(const QModelIndex &) const
{
    return m_packages.size();
}

QSize BackgroundListModel::bestSize(Plasma::Package *package) const
{
    if (m_sizeCache.contains(package)) {
        return m_sizeCache.value(package);
    }

    QString image = package->filePath("preferred");
    if (image.isEmpty()) {
        return QSize();
    }

    KFileMetaInfo info(image, QString(), KFileMetaInfo::TechnicalInfo);
    QSize size(info.item("http://freedesktop.org/standards/xesam/1.0/core#width").value().toInt(),
               info.item("http://freedesktop.org/standards/xesam/1.0/core#height").value().toInt());

    //backup solution if strigi does not work
    if (size.width() == 0 || size.height() == 0) {
        kDebug() << "fall back to QImage, check your strigi";
        size = QImage(image).size();
    }

    const_cast<BackgroundListModel *>(this)->m_sizeCache.insert(package, size);
    return size;
}

QVariant BackgroundListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    if (index.row() >= m_packages.size()) {
        return QVariant();
    }

    Plasma::Package *b = package(index.row());
    if (!b) {
        return QVariant();
    }

    switch (role) {
    case Qt::DisplayRole: {
        QString title = b->metadata().name();

        if (title.isEmpty()) {
            return QFileInfo(b->filePath("preferred")).completeBaseName();
        }

        return title;
    }
    break;

    case BackgroundDelegate::ScreenshotRole: {
        if (m_previews.contains(b)) {
            return m_previews.value(b);
        }

        KUrl file(b->filePath("preferred"));

        if (file.isValid()) {
            KIO::PreviewJob* job = KIO::filePreview(KUrl::List() << file,
                                                    BackgroundDelegate::SCREENSHOT_SIZE,
                                                    BackgroundDelegate::SCREENSHOT_SIZE);

            connect(job, SIGNAL(gotPreview(const KFileItem&, const QPixmap&)),
                    this, SLOT(showPreview(const KFileItem&, const QPixmap&)));
            connect(job, SIGNAL(failed(const KFileItem&)),
                    this, SLOT(previewFailed(const KFileItem&)));
            const_cast<BackgroundListModel *>(this)->m_previewJobs.insert(file, QPersistentModelIndex(index));
        }

        QPixmap pix(BackgroundDelegate::SCREENSHOT_SIZE, BackgroundDelegate::SCREENSHOT_SIZE);
        pix.fill(Qt::transparent);
        const_cast<BackgroundListModel *>(this)->m_previews.insert(b, pix);
        return pix;
    }
    break;

    case BackgroundDelegate::AuthorRole:
        return b->metadata().author();
    break;

    case BackgroundDelegate::ResolutionRole:{
        QSize size = bestSize(b);

        if (size.isValid()) {
            return QString("%1x%2").arg(size.width()).arg(size.height());
        }

        return QString();
    }
    break;

    default:
        return QVariant();
    break;
    }
}

void BackgroundListModel::showPreview(const KFileItem &item, const QPixmap &preview)
{
    QPersistentModelIndex index = m_previewJobs.value(item.url());
    m_previewJobs.remove(item.url());

    if (!index.isValid()) {
        return;
    }

    Plasma::Package *b = package(index.row());
    if (!b) {
        return;
    }

    m_previews.insert(b, preview);
    static_cast<WeatherWallpaper*>(m_listener)->updateScreenshot(index);
}

void BackgroundListModel::previewFailed(const KFileItem &item)
{
    m_previewJobs.remove(item.url());
}

Plasma::Package* BackgroundListModel::package(int index) const
{
    return m_packages.at(index);
}

void BackgroundListModel::initProgressDialog(KProgressDialog *progress)
{
    progress->setAllowCancel(false);
    progress->setModal(true);
    progress->setLabelText(i18n("Finding images for the wallpaper slideshow."));
    progress->progressBar()->setRange(0, 0);
}

QList<Plasma::Package *> BackgroundListModel::findAllBackgrounds(Plasma::Wallpaper *structureParent,
                                                                 const BackgroundListModel *container,
                                                                 const QString &path, float ratio,
                                                                 KProgressDialog *progress)
{
    KProgressDialog *myProgress = 0;
    if (!progress) {
        myProgress = progress = new KProgressDialog;
        initProgressDialog(myProgress);
    }

    //kDebug() << "looking for" << path;
    QList<Plasma::Package *> res;

    // get all packages in this directory
    //kDebug() << "getting packages";
    QStringList packages = Plasma::Package::listInstalledPaths(path);
    QSet<QString> validPackages;
    foreach (const QString &packagePath, packages) {
        QCoreApplication::processEvents();
        progress->setLabelText(i18n("Finding images for the wallpaper slideshow.") + "\n\n" +
                               i18n("Testing %1 for a Wallpaper package", packagePath));
        Plasma::PackageStructure::Ptr structure = Plasma::Wallpaper::packageStructure(structureParent);
        Plasma::Package *pkg = new Plasma::Package(path + packagePath, structure);
        if (pkg->isValid() && (!container || !container->contains(pkg->path()))) {
            progress->setLabelText(i18n("Finding images for the wallpaper slideshow.") + "\n\n" +
                                   i18n("Adding wallpaper package in %1", packagePath));
            res.append(pkg);
            //kDebug() << "    adding valid package:" << packagePath;
            validPackages << packagePath;
        } else {
            delete pkg;
        }
    }

    // search normal wallpapers
    //kDebug() << "listing normal files";
    QDir dir(path);
    QStringList filters;
    filters << "*.png" << "*.jpeg" << "*.jpg" << "*.svg" << "*.svgz";
    dir.setNameFilters(filters);
    dir.setFilter(QDir::Files | QDir::Hidden | QDir::Readable);
    QFileInfoList files = dir.entryInfoList();
    foreach (const QFileInfo &wp, files) {
        QCoreApplication::processEvents();
        if (!container || !container->contains(wp.filePath())) {
            //kDebug() << "     adding image file" << wp.filePath();
            progress->setLabelText(i18n("Finding images for the wallpaper slideshow.") + "\n\n" +
                                   i18n("Adding image %1", wp.filePath()));
            Plasma::PackageStructure::Ptr structure = Plasma::Wallpaper::packageStructure(structureParent);
            res.append(new Plasma::Package(wp.filePath(), structure));
        }
    }

    // now recurse the dirs, skipping ones we found packages in
    //kDebug() << "recursing dirs";
    dir.setFilter(QDir::AllDirs | QDir::Readable);
    files = dir.entryInfoList();

    foreach (const QFileInfo &wp, files) {
        QCoreApplication::processEvents();
        QString name = wp.fileName();
        if (name != "." && name != ".." && !validPackages.contains(wp.fileName())) {
            //kDebug() << "    " << name << wp.filePath();
            res += findAllBackgrounds(structureParent, container, wp.filePath(), ratio, progress);
        }
    }

    //kDebug() << "completed.";
    delete myProgress;
    return res;
}

void BackgroundListModel::setWallpaperSize(QSize size)
{
    m_size = size;
}

void BackgroundListModel::setResizeMethod(Plasma::Wallpaper::ResizeMethod resizeMethod)
{
    m_resizeMethod = resizeMethod;
}

#include "backgroundlistmodel.moc"


