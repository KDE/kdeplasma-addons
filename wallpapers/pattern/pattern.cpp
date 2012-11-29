/*
Copyright 2008 Will Stephenson <wstephenson@kde.org>
Copyright 2011 Reza Fatahilah Shah <rshah0385@kireihana.com>

Inspired by kdesktop (C) 1999 Geert Jansen <g.t.jansen@stud.tue.nl>

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of
the License or (at your option) version 3 or any later version
accepted by the membership of KDE e.V. (or its successor approved
by the membership of KDE e.V.), which shall act as a proxy
defined in Section 14 of version 3 of the license.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "pattern.h"

#include "qimageblitz.h"

#include <QFileInfo>
#include <QPainter>
#include <QTimer>

#include <KDebug>
#include <KStandardDirs>
#include "backgroundlistmodel.h"
#include "backgrounddelegate.h"

#define FOREGROUND_COLOR_CONFIG_KEY "ForegroundColor"
#define BACKGROUND_COLOR_CONFIG_KEY "BackgroundColor"
#define PATTERN_CONFIG_KEY "Pattern"
#define COMMENT_CONFIG_KEY "Comment"
#define FILE_CONFIG_KEY "File"
#define PATTERN_CONFIG_GROUP "KDE Desktop Pattern"
#define PATTERN_RESOURCE_TYPE "dtop_pattern"

K_EXPORT_PLASMA_WALLPAPER(pattern, PatternWallpaper)

PatternWallpaper::PatternWallpaper(QObject * parent, const QVariantList & args )
    : Plasma::Wallpaper(parent, args),
      m_dirs(KGlobal::dirs()),
      m_model(0)
{
    m_dirs->addResourceType(PATTERN_RESOURCE_TYPE, "data" , "plasma_wallpaper_pattern/patterns");
}

PatternWallpaper::~PatternWallpaper()
{

}

void PatternWallpaper::init(const KConfigGroup & config)
{
    m_fgColor = config.readEntry(FOREGROUND_COLOR_CONFIG_KEY, QColor(Qt::white));
    m_bgColor = config.readEntry(BACKGROUND_COLOR_CONFIG_KEY, QColor(Qt::black));
    m_patternName = config.readEntry(PATTERN_CONFIG_KEY, QString());
    loadPattern();
    emit update(boundingRect());
}

void PatternWallpaper::loadPattern()
{
    if (!m_patternName.isEmpty()) {
        m_pattern = generatePattern(m_patternName, m_fgColor, m_bgColor);
    }
}

QWidget * PatternWallpaper::createConfigurationInterface(QWidget * parent)
{
    QWidget * configWidget = new QWidget(parent);

    m_ui.setupUi(configWidget);
    m_ui.m_fgColor->setColor(m_fgColor);
    m_ui.m_bgColor->setColor(m_bgColor);

    m_model = new BackgroundListModel(this, configWidget);
    m_model->setWallpaperSize(targetSizeHint().toSize());
    m_model->reload();

    QTimer::singleShot(0, this, SLOT(setConfigurationInterfaceModel()));
    m_ui.m_pattern->setItemDelegate(new BackgroundDelegate(m_ui.m_pattern));

    m_ui.m_pattern->setMinimumWidth((BackgroundDelegate::SCREENSHOT_SIZE + BackgroundDelegate::MARGIN * 2 +
                                        BackgroundDelegate::BLUR_INCREMENT) * 3 +
                                        m_ui.m_pattern->spacing() * 4 +
                                        QApplication::style()->pixelMetric(QStyle::PM_ScrollBarExtent) +
                                        QApplication::style()->pixelMetric(QStyle::PM_DefaultFrameWidth) * 2 + 7);
    m_ui.m_pattern->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);

    connect(m_ui.m_fgColor, SIGNAL(changed(QColor)), SLOT(widgetChanged()));
    connect(m_ui.m_bgColor, SIGNAL(changed(QColor)), SLOT(widgetChanged()));

    connect(this, SIGNAL(settingsChanged(bool)), parent, SLOT(settingsChanged(bool)));
    return configWidget;
}

QPixmap PatternWallpaper::generatePattern(QImage &image) const
{
    return QPixmap::fromImage(Blitz::flatten(image, m_fgColor, m_bgColor));
}

QPixmap PatternWallpaper::generatePattern(const QString &patternFile, const QColor &fg, const QColor &bg) const
{
    QImage img;
    const QString path = m_dirs->findResource(PATTERN_RESOURCE_TYPE, patternFile);

    if (!img.load(path, 0)) {
        kDebug() << "pattern" << patternFile << "at" << path << "failed to load";
        return QPixmap();
    }

    return QPixmap::fromImage(Blitz::flatten(img, fg, bg));
}

void PatternWallpaper::save(KConfigGroup & config)
{
    config.writeEntry(FOREGROUND_COLOR_CONFIG_KEY, m_fgColor);
    config.writeEntry(BACKGROUND_COLOR_CONFIG_KEY, m_bgColor);
    config.writeEntry(PATTERN_CONFIG_KEY, m_patternName);
}

void PatternWallpaper::paint(QPainter * painter, const QRectF & exposedRect)
{
    if (!m_pattern.isNull()) {
        painter->drawTiledPixmap(exposedRect, m_pattern, exposedRect.topLeft());
    }
}

void PatternWallpaper::widgetChanged()
{
    const QColor newFgColor = m_ui.m_fgColor->color();
    const QColor newBgColor = m_ui.m_bgColor->color();
    const bool updateThumbs = (m_fgColor != newFgColor) || (m_bgColor != newBgColor);

    m_fgColor = newFgColor;
    m_bgColor = newBgColor;

    if (updateThumbs) {
        m_model->reload();
    }

    loadPattern();
    emit settingsChanged(true);
    emit update(boundingRect());
}

void PatternWallpaper::pictureChanged(const QModelIndex &index)
{
    if (index.row() == -1 || !m_model) {
        return;
    }

    KConfig *config = m_model->kconfig(index.row());
    if (!config) {
        return;
    }

    KConfigGroup patternConfig(config, PATTERN_CONFIG_GROUP);

    m_patternName = patternConfig.readEntry("File", QString());
    kDebug() << "Pattern changed to =" << m_patternName;
    emit settingsChanged(true);
    emit update(boundingRect());
}

void PatternWallpaper::setConfigurationInterfaceModel()
{
    m_ui.m_pattern->setModel(m_model);
    connect(m_ui.m_pattern->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)), this, SLOT(pictureChanged(QModelIndex)));

    QModelIndex index = m_model->indexOf(m_patternName);
    if (index.isValid()) {
        m_ui.m_pattern->setCurrentIndex(index);
    }
}

void PatternWallpaper::updateScreenshot(QPersistentModelIndex index)
{
    m_ui.m_pattern->update(index);
}

#include "pattern.moc"

// vim: sw=4 sts=4 et tw=100