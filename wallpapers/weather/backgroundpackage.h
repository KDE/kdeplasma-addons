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

#ifndef BACKGROUNDPACKAGE_H
#define BACKGROUNDPACKAGE_H

#include <memory>
#include <QImage>
#include <QPersistentModelIndex>
#include <QPixmap>
#include <QSize>
#include <QThread>
#include <QVariant>

#include <ThreadWeaver/Job>

#include <Plasma/Package>
#include <Plasma/Wallpaper>

namespace Plasma { class PackageMetadata; }

class DummyObject : public QObject
{
Q_OBJECT
signals:
    void done(ThreadWeaver::Job *);
};

class Background : public QObject
{
Q_OBJECT
public:
    static const int SCREENSHOT_HEIGHT = 60;

    virtual ~Background();

    virtual QString path() const = 0;
    virtual QString findBackground(const QSize &resolution,
                                   Plasma::Wallpaper::ResizeMethod method) const = 0;
    virtual QPixmap screenshot() const = 0;
    virtual bool screenshotGenerationStarted() const = 0;
    virtual void generateScreenshot(QPersistentModelIndex index) const = 0;
    virtual QString title() const = 0;
    virtual QString author() const = 0;
    virtual QString email() const = 0;
    virtual QString license() const = 0;
    virtual QSize bestSize(const QSize &resolution,
                            Plasma::Wallpaper::ResizeMethod method) const = 0;

    virtual bool isValid() const = 0;

protected:
    static QImage defaultScreenshot();
    static QImage createScreenshot(const QString &path, float ratio);
    friend class ResizeThread;
};

class BackgroundPackage : public Background,
                          public Plasma::Package
{
Q_OBJECT
public:
    BackgroundPackage(const QString &path, float ratio);

    virtual QString path() const;
    virtual QString findBackground(const QSize &resolution,
                                   Plasma::Wallpaper::ResizeMethod method) const;
    virtual QPixmap screenshot() const;
    virtual bool screenshotGenerationStarted() const;
    virtual void generateScreenshot(QPersistentModelIndex index) const;
    virtual QString author() const;
    virtual QString title() const;
    virtual QString email() const;
    virtual QString license() const;
    virtual QSize bestSize(const QSize &resolution,
                           Plasma::Wallpaper::ResizeMethod method) const;
    virtual bool isValid() const;
private:
    QString resString(const QSize &size) const;
    QSize resSize(const QString &res) const;

    float distance(const QSize &size,
                   const QSize &desired,
                   Plasma::Wallpaper::ResizeMethod method) const;

    QString m_path;
    float m_ratio;
    mutable QPixmap m_screenshot;
};

class BackgroundFile : public Background
{
Q_OBJECT
public:
    BackgroundFile(const QString &file, float ratio);
    virtual ~BackgroundFile();

    virtual QString path() const;
    virtual QString findBackground(const QSize &resolution,
                                   Plasma::Wallpaper::ResizeMethod method) const;
    virtual bool screenshotGenerationStarted() const;
    virtual void generateScreenshot(QPersistentModelIndex index) const;
    virtual QPixmap screenshot() const;
    virtual QString author() const;
    virtual QString title() const;
    virtual QString email() const;
    virtual QString license() const;
    virtual QSize bestSize(const QSize &resolution,
                           Plasma::Wallpaper::ResizeMethod method) const;
    virtual bool isValid() const;
private:
    QString m_file;
    float m_ratio;
    mutable QSize m_sizeCache;

    mutable bool m_resizer_started;
    mutable QPixmap m_screenshot;
private slots:
    void updateScreenshot(ThreadWeaver::Job *);
signals:
    void screenshotDone(QPersistentModelIndex index);
};

#endif // BACKGROUNDPACKAGE_H
