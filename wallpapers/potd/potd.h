/*
 *   Copyright 2011 Aaron Seigo <aseigo@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
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

#ifndef COLOR_HEADER
#define COLOR_HEADER

#include <Plasma/Wallpaper>
#include <Plasma/DataEngine>

#include "ui_config.h"

class PoTD : public Plasma::Wallpaper
{
    Q_OBJECT

public:
    PoTD(QObject* parent, const QVariantList& args);

    void save(KConfigGroup &config);
    void paint(QPainter* painter, const QRectF& exposedRect);
    QWidget *createConfigurationInterface(QWidget* parent);

public Q_SLOTS:
    void wallpaperRendered(const QImage &image);
    void dataUpdated(const QString &source, const Plasma::DataEngine::Data &data);

Q_SIGNALS:
    void settingsChanged(bool modified);

protected:
    virtual void init(const KConfigGroup &config);

protected slots:
    void settingsModified();

private:
    Ui::Configuration m_ui;
    Plasma::DataEngine::Data m_providers;
    QString m_provider;
    QString m_configProvider;
    QImage m_image;
};

K_EXPORT_PLASMA_WALLPAPER(color, PoTD)

#endif
