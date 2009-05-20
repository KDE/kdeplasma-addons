/*
Copyright 2008 Will Stephenson <wstephenson@kde.org>

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
#include <QImage>
#include <QPainter>
#include <QPixmap>

#include <KDebug>
#include <KGlobal>
#include <KStandardDirs>


#define FOREGROUND_COLOR_CONFIG_KEY "ForegroundColor"
#define BACKGROUND_COLOR_CONFIG_KEY "BackgroundColor"
#define PATTERN_CONFIG_KEY "Pattern"
#define COMMENT_CONFIG_KEY "Comment"
#define FILE_CONFIG_KEY "File"
#define PATTERN_CONFIG_GROUP "KDE Desktop Pattern"
#define PATTERN_RESOURCE_TYPE "dtop_pattern"

PatternWallpaper::PatternWallpaper(QObject * parent, const QVariantList & args ) : Plasma::Wallpaper(parent, args), m_dirs(KGlobal::dirs())
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
        const QString pattern = m_dirs->findResource(PATTERN_RESOURCE_TYPE, m_patternName);
        QImage m_patternImage;

        if (m_patternImage.load(pattern, 0)) {
            m_patternImage = Blitz::flatten(m_patternImage, m_fgColor, m_bgColor);
            m_pattern = QPixmap::fromImage(m_patternImage);
        } else {
            kDebug() << "pattern" << m_patternName << "at" << pattern << "failed to load";
        }
    }
}

QWidget * PatternWallpaper::createConfigurationInterface(QWidget * parent)
{
    QWidget * configWidget = new QWidget(parent);
    m_ui.setupUi(configWidget);
    m_ui.m_fgColor->setColor(m_fgColor);
    m_ui.m_bgColor->setColor(m_bgColor);

    // populate the combo box
    QStringList patterns = m_dirs->findAllResources(PATTERN_RESOURCE_TYPE, QLatin1String("*.desktop"), KStandardDirs::NoDuplicates);
    qSort(patterns);

    int configuredPatternIndex = -1;
    int i = 0;
    foreach (const QString& pattern, patterns) {
        KConfig cfg(pattern);
        KConfigGroup patternConfig(&cfg, PATTERN_CONFIG_GROUP);
        QString patternComment;
        QString patternFile;

        if (patternConfig.hasKey(FILE_CONFIG_KEY)) {
            patternFile = patternConfig.readEntry(FILE_CONFIG_KEY, QString());
            if (patternFile.isEmpty()) {
                continue;
            }
        } else {
            continue;
        }

        if (patternFile == m_patternName) {
            configuredPatternIndex = i;
        }

        if (patternConfig.hasKey(COMMENT_CONFIG_KEY)) {
            patternComment = patternConfig.readEntry(COMMENT_CONFIG_KEY);
        } else {
            QFileInfo fi(pattern);
            patternComment = fi.baseName();
        }

        m_ui.m_pattern->addItem(patternComment, patternFile);
        ++i;
    }
    if (configuredPatternIndex != -1) {
        m_ui.m_pattern->setCurrentIndex(configuredPatternIndex);
    }

    connect(m_ui.m_fgColor, SIGNAL(changed(const QColor&)), SLOT(widgetChanged()));
    connect(m_ui.m_bgColor, SIGNAL(changed(const QColor&)), SLOT(widgetChanged()));
    connect(m_ui.m_pattern, SIGNAL(currentIndexChanged(int)), SLOT(widgetChanged()));

    connect(this, SIGNAL(settingsChanged(bool)), parent, SLOT(settingsChanged(bool)));
    return configWidget;
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
    m_fgColor = m_ui.m_fgColor->color();
    m_bgColor = m_ui.m_bgColor->color();
    if (m_ui.m_pattern->count()) {
        m_patternName = m_ui.m_pattern->itemData(m_ui.m_pattern->currentIndex()).toString();
    }
    loadPattern();
    emit settingsChanged(true);
    emit update(boundingRect());
}

#include "pattern.moc"

// vim: sw=4 sts=4 et tw=100
