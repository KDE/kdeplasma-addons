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

#ifndef PATTERN_H
#define PATTERN_H

#include <Plasma/Wallpaper>
#include <QColor>

#include "ui_config.h"

class KStandardDirs;

class PatternWallpaper : public Plasma::Wallpaper
{
Q_OBJECT
public:
    PatternWallpaper(QObject * parent, const QVariantList & args);
    ~PatternWallpaper();

    QWidget * createConfigurationInterface(QWidget * parent);
    void save(KConfigGroup & config);
    void paint(QPainter * painter, const QRectF & exposedRect);

    signals:
        void settingsChanged(bool);

protected:
    void init(const KConfigGroup & config);
private slots:
    void widgetChanged();
private:
    void loadPattern();

    Ui_PatternSettingsWidget m_ui;
    QColor m_fgColor;
    QColor m_bgColor;
    QString m_patternName;
    QPixmap m_pattern;
    KStandardDirs * m_dirs;
};

K_EXPORT_PLASMA_WALLPAPER(pattern, PatternWallpaper)

#endif //PLASMA_PLUGIN_WALLPAPER_PATTERN_H
