/*
  Copyright (c) 2007 Paolo Capriotti <p.capriotti@gmail.com>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.
*/

#include "backgroundlistmodel.h"

#include <QFile>
#include <QDir>

#include <KDebug>
#include <KFileMetaInfo>
#include <KGlobal>
#include <KIO/PreviewJob>
#include <KProgressDialog>
#include <KStandardDirs>

#include <Plasma/Package>
#include <Plasma/PackageStructure>

#include "backgrounddelegate.h"
#include "virus.h"

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
    QModelIndex index;
    while ((index = indexOf(path)).isValid()) {
        beginRemoveRows(QModelIndex(), index.row(), index.row());
        Plasma::Package *package = m_packages.at(index.row());
        m_packages.removeAt(index.row());
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
    const QStringList dirs = KGlobal::dirs()->findDirs("wallpaper", "");
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
        const QStringList backgrounds = findAllBackgrounds(m_structureParent, this, dirs);
        foreach (const QString &background, backgrounds) {
            tmp << new Plasma::Package(background, Plasma::Wallpaper::packageStructure(m_structureParent));
        }
    }

    // add new files to dirwatch
    foreach (Plasma::Package *b, tmp) {
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

QStringList BackgroundFinder::papersFound() const
{
    return m_papersFound;
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

QModelIndex BackgroundListModel::indexOf(const QString &path) const
{
    for (int i = 0; i < m_packages.size(); i++) {
        // packages will end with a '/', but the path passed in may not
        QString package = m_packages[i]->path();
        if (package.at(package.length() - 1) == '/') {
            package.truncate(package.length() - 1);
        }

        if (path.startsWith(package)) {
            // FIXME: ugly hack to make a difference between local files in the same dir
            // package->path does not contain the actual file name
            if ((!m_packages[i]->structure()->contentsPrefix().isEmpty()) ||
                (path == m_packages[i]->filePath("preferred"))) {
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
    return m_packages.size();
}

QSize BackgroundListModel::bestSize(Plasma::Package *package) const
{
    if (m_sizeCache.contains(package)) {
        return m_sizeCache.value(package);
    }

    const QString image = package->filePath("preferred");
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
    static_cast<Virus*>(m_listener)->updateScreenshot(index);
}

void BackgroundListModel::previewFailed(const KFileItem &item)
{
    m_previewJobs.remove(item.url());
}

Plasma::Package* BackgroundListModel::package(int index) const
{
    return m_packages.at(index);
}

QStringList BackgroundListModel::findAllBackgrounds(Plasma::Wallpaper *structureParent,
                                                    const BackgroundListModel *container,
                                                    const QStringList &p)
{
    //TODO: put this in a thread so that it can run in the background without blocking
    QEventLoop localEventLoop;
    BackgroundFinder finder(structureParent, container, p, &localEventLoop);
    connect(&finder, SIGNAL(finished()), &localEventLoop, SLOT(quit()));
    QTimer::singleShot(0, &finder, SLOT(start()));
    localEventLoop.exec();
    return finder.papersFound();
}

BackgroundFinder::BackgroundFinder(Plasma::Wallpaper *structureParent,
                                   const BackgroundListModel *container,
                                   const QStringList &paths,
                                   QEventLoop *eventLoop)
    : QObject(0),
      m_structureParent(structureParent),
      m_container(container),
      m_paths(paths),
      m_eventLoop(eventLoop)
{
}

void BackgroundFinder::start()
{
    KProgressDialog *progress = new KProgressDialog;
    progress->setAllowCancel(false);
    progress->setModal(true);
    progress->setLabelText(i18n("Finding images for the wallpaper slideshow."));
    progress->progressBar()->setRange(0, 0);

    QSet<QString> suffixes;
    suffixes << "png" << "jpeg" << "jpg" << "svg" << "svgz";

    QDir dir;
    dir.setFilter(QDir::AllDirs | QDir::Files | QDir::Hidden | QDir::Readable);

    int count = 0;
    int allCount = 0;
    bool setLabel = true;
    while (!m_paths.isEmpty()) {
        QString path = m_paths.takeLast();
        //kDebug() << "doing" << path;
        dir.setPath(path);
        const QFileInfoList files = dir.entryInfoList();
        foreach (const QFileInfo &wp, files) {
            if (wp.isDir()) {
                //kDebug() << "directory" << wp.fileName() << validPackages.contains(wp.fileName());
                QString name = wp.fileName();
                if (name == "." || name == "..") {
                    // do nothing
                } else if(QFile::exists(wp.filePath() + "/metadata.desktop")) {
                    Plasma::PackageStructure::Ptr structure = Plasma::Wallpaper::packageStructure(m_structureParent);
                    Plasma::Package pkg(wp.filePath(), structure);

                    if (pkg.isValid() && (!m_container || !m_container->contains(pkg.path()))) {
                        if (setLabel) {
                            progress->setLabelText(i18n("Finding images for the wallpaper slideshow.") + "\n\n" +
                                                   i18n("Adding wallpaper package in %1", name));
                        }

                        ++count;
                        m_papersFound << pkg.path();
                        //kDebug() << "gots a" << wp.filePath();
                    } else {
                        m_paths.append(wp.filePath());
                    }
                } else {
                    m_paths.append(wp.filePath());
                }
            } else if (suffixes.contains(wp.suffix().toLower()) && (!m_container || !m_container->contains(wp.filePath()))) {
                //kDebug() << "adding" << wp.filePath() << setLabel;
                if (setLabel) {
                    progress->setLabelText(i18n("Finding images for the wallpaper slideshow.") + "\n\n" +
                                           i18n("Adding image %1", wp.filePath()));
                    setLabel = false;
                }
                //kDebug() << "     adding image file" << wp.filePath();
                ++count;
                m_papersFound << wp.filePath();
            }

            ++allCount;
            if (allCount % 10 == 0) {
                m_eventLoop->processEvents(QEventLoop::ExcludeUserInputEvents);
                if (progress->isVisible() && count % 10) {
                    setLabel = true;
                }
            }
        }
    }

    delete progress;
    emit finished();
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


