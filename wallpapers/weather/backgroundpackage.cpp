/*
 *   Copyright (c) 2007 Paolo Capriotti <p.capriotti@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License version 2,
 *   or (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "backgroundpackage.h"
#include <cmath>
// <cmath> does not define fabs (by the standard, even if it does with gcc)
#include <math.h>
#include <float.h> // FLT_MAX

#include <QFileInfo>
#include <QPainter>
#include <KDebug>
#include <KLocalizedString>
#include <KStandardDirs>
#include <KSvgRenderer>

#include <ThreadWeaver/Weaver>

#include <Plasma/PackageMetadata>

#include <kfilemetainfo.h>

using namespace Plasma;

class ResizeThread : public ThreadWeaver::Job
{
public:
    ResizeThread(const QString &path, float ratio, QObject *parent = 0);
    virtual ~ResizeThread();

    virtual void start(QPersistentModelIndex index);
    virtual void run();

    QImage result() const;
    QPersistentModelIndex index() const;
    bool isInitialized() const;
private:
    QString m_path;
    QImage m_result;
    float m_ratio;
    QPersistentModelIndex m_index;
};

ResizeThread::ResizeThread(const QString &path, float ratio, QObject *parent)
    : ThreadWeaver::Job(parent),
      m_path(path),
      m_ratio(ratio)
{
}

ResizeThread::~ResizeThread() {
}

void ResizeThread::start(QPersistentModelIndex index)
{
    m_index = index;
    ThreadWeaver::Weaver::instance()->enqueue(this);
}

bool ResizeThread::isInitialized() const
{
    return m_index.isValid();
}

void ResizeThread::run()
{
    m_result = Background::createScreenshot(m_path, m_ratio);
}

QImage ResizeThread::result() const
{
    if (isFinished()) {
        return m_result;
    } else {
        return QImage();
    }
}

QPersistentModelIndex ResizeThread::index() const
{
    return m_index;
}

Background::~Background()
{
}

QImage Background::createScreenshot(const QString &path, float ratio)
{
    if (path.endsWith("svg") || path.endsWith("svgz")) {
        KSvgRenderer renderer(path);
        QImage img(QSize(int(SCREENSHOT_HEIGHT * ratio), SCREENSHOT_HEIGHT),
                   QImage::Format_ARGB32_Premultiplied);
        img.fill(0);
        QPainter p(&img);
        renderer.render(&p);
        return img;
    } else {
        QImage img(path);
        if (!img.isNull()) {
            return img.scaled(int(SCREENSHOT_HEIGHT * ratio),
                            SCREENSHOT_HEIGHT,
                            Qt::KeepAspectRatio);
        } else {
            return defaultScreenshot();
        }
    }

}

QImage Background::defaultScreenshot()
{
    static QImage defaultScreenshotImage;

    if (defaultScreenshotImage.isNull()) {
        QImage img(QSize(SCREENSHOT_HEIGHT, SCREENSHOT_HEIGHT), QImage::Format_ARGB32_Premultiplied);
        img.fill(Qt::white);
        QPainter p(&img);
        p.drawText(QRect(0, 0, SCREENSHOT_HEIGHT, SCREENSHOT_HEIGHT),
                   Qt::AlignHCenter | Qt::AlignVCenter,
                   "Preview\nnot\navailable");
        defaultScreenshotImage = img;
    }
    return defaultScreenshotImage;
}


BackgroundPackage::BackgroundPackage(const QString &path, float ratio)
    : Package(path, Plasma::Wallpaper::packageStructure()),
      m_path(path),
      m_ratio(ratio)
{
}

QString BackgroundPackage::resString(const QSize &size) const
{
    return QString::number(size.width()) + 'x' + QString::number(size.height());
}

QSize BackgroundPackage::resSize(const QString &str) const
{
    int index = str.indexOf('x');
    if (index != -1) {
        return QSize(str.left(index).toInt(),
                     str.mid(index + 1).toInt());
    } else {
        return QSize();
    }
}

QString BackgroundPackage::findBackground(const QSize &size,
                                          Plasma::Wallpaper::ResizeMethod method) const
{
    QStringList images = entryList("images");
    if (images.empty()) {
        return QString();
    }

    //kDebug() << "wanted" << size;

    // choose the nearest resolution
    float best = FLT_MAX;
    QString bestImage;
    foreach (const QString &entry, images) {
        QSize candidate = resSize(QFileInfo(entry).baseName());
        if (candidate == QSize()) {
            continue;
        }

        double dist = distance(candidate, size, method);
        //kDebug() << "candidate" << candidate << "distance" << dist;
        if (bestImage.isNull() || dist < best) {
            bestImage = filePath("images", entry);
            best = dist;
            //kDebug() << "best" << bestImage;
            if (dist == 0) {
                break;
            }
        }
    }

    //kDebug() << "best image" << bestImage;
    return bestImage;
}

float BackgroundPackage::distance(const QSize& size,
                                   const QSize& desired,
                                   Plasma::Wallpaper::ResizeMethod method) const
{
    // compute difference of areas
    float delta = size.width() * size.height() -
                  desired.width() * desired.height();
    // scale down to about 1.0
    delta /= ((desired.width() * desired.height())+(size.width() * size.height()))/2;


    switch (method) {
    case Plasma::Wallpaper::ScaledResize: {
        // Consider first the difference in aspect ratio,
        // then in areas. Prefer scaling down.
        float deltaRatio = 1.0;
        if (size.height() > 0 && desired.height() > 0) {
            deltaRatio = float(size.width()) / float(size.height()) -
                         float(desired.width()) / float(desired.height());
        }
        return fabs(deltaRatio) * 3.0 + (delta >= 0.0 ? delta : -delta + 5.0);
    }
    case Plasma::Wallpaper::ScaledAndCroppedResize:
        // Difference of areas, slight preference to scale down
        return delta >= 0.0 ? delta : -delta + 2.0;
    default:
        // Difference in areas
        return fabs(delta);
    }
}

QPixmap BackgroundPackage::screenshot() const
{
    if (m_screenshot.isNull()) {
        QString screenshotPath = filePath("screenshot");
        if (!screenshotPath.isEmpty()) {
            QImage img = createScreenshot(screenshotPath, m_ratio);
            m_screenshot = QPixmap::fromImage(img);
        }
    }

    return m_screenshot;
}

bool BackgroundPackage::screenshotGenerationStarted() const
{
    return true;
}

void BackgroundPackage::generateScreenshot(QPersistentModelIndex) const
{
}

QString BackgroundPackage::title() const
{
    Plasma::PackageMetadata md = metadata();
    QString title = md.name();
    if (title.isEmpty()) {
        title = md.pluginName();
        title.replace("_", " ");
    }
    return title;
}

QString BackgroundPackage::author() const
{
    return metadata().author();
}

QString BackgroundPackage::email() const
{
    return metadata().email();
}

QString BackgroundPackage::license() const
{
    return metadata().license();
}

QSize BackgroundPackage::bestSize(const QSize &resolution, Plasma::Wallpaper::ResizeMethod method) const
{
    QStringList images = entryList("images");
    if (images.empty()) {
        return QSize();
    }

    // choose the nearest resolution
    float best = FLT_MAX;
    QSize bestSize = QSize();
    foreach (const QString &entry, images) {
        QSize candidate = resSize(QFileInfo(entry).baseName());
        if (candidate == QSize()) {
            continue;
        }

        double dist = distance(candidate, resolution, method);

        if (dist < best || !bestSize.isValid()) {
            best = dist;
            bestSize = candidate;

            if (dist == 0) {
                break;
            }
        }
    }

    return bestSize;
}

bool BackgroundPackage::isValid() const
{
    return Package::isValid();
}

QString BackgroundPackage::path() const
{
    return m_path;
}


BackgroundFile::BackgroundFile(const QString &file, float ratio)
: m_file(file)
, m_ratio(ratio)
, m_resizer_started(false)
{
}

BackgroundFile::~BackgroundFile()
{
}

QString BackgroundFile::findBackground(const QSize &,
                                       Plasma::Wallpaper::ResizeMethod) const
{
    return m_file;
}

QPixmap BackgroundFile::screenshot() const
{
    return m_screenshot;
}

bool BackgroundFile::screenshotGenerationStarted() const
{
    return m_resizer_started;
}

void BackgroundFile::generateScreenshot(QPersistentModelIndex index) const
{
    ResizeThread *resizer = new ResizeThread(m_file, m_ratio);
    connect(resizer, SIGNAL(done(ThreadWeaver::Job *)),
            this, SLOT(updateScreenshot(ThreadWeaver::Job *)));
    m_resizer_started = true;
    resizer->start(index);
}

void BackgroundFile::updateScreenshot(ThreadWeaver::Job *job)
{
    ResizeThread *resizer = static_cast<ResizeThread *>(job);
    m_screenshot = QPixmap::fromImage(resizer->result());
    emit screenshotDone(resizer->index());
    resizer->deleteLater();
}

//TODO: impl
QString BackgroundFile::author() const
{
    return QString();
}

QString BackgroundFile::title() const
{
    return QFileInfo(m_file).completeBaseName();
}

//TODO: impl
QString BackgroundFile::email() const
{
    return QString();
}

//TODO: impl
QString BackgroundFile::license() const
{
    return QString();
}

QSize BackgroundFile::bestSize(const QSize &resolution, Plasma::Wallpaper::ResizeMethod method) const
{
    Q_UNUSED(resolution)
    Q_UNUSED(method)

    if (!m_sizeCache.isValid()) {
        KFileMetaInfo info(m_file, QString(), KFileMetaInfo::TechnicalInfo);
        m_sizeCache = QSize(info.item("http://freedesktop.org/standards/xesam/1.0/core#width").value().toInt(),
                            info.item("http://freedesktop.org/standards/xesam/1.0/core#height").value().toInt());

        //backup solution if strigi does not work
        if (m_sizeCache.width() == 0 || m_sizeCache.height() == 0) {
            kDebug() << "fall back to QImage, check your strigi";
            m_sizeCache = QImage(m_file).size();
        }
    }

    return m_sizeCache;
}

//TODO: impl
bool BackgroundFile::isValid() const
{
    return true;
}

QString BackgroundFile::path() const
{
    return m_file;
}
