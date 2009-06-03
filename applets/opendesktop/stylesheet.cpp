/*
    Copyright 2009 by Sebastian Kügler <sebas@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

//Qt
#include <QFile>
#include <QPalette>

// KDE
#include <KDirWatch>
#include <KGlobalSettings>
#include <KStandardDirs>

// Plasma
#include <Plasma/Theme>

// own
#include "stylesheet.h"

using namespace Plasma;

StyleSheet::StyleSheet(QObject *parent)
    : QObject(parent)
{
    //m_cssFile = "/home/sebas/kdesvn/src/attica/plasma/opendesktop/user.css"; // For debugging quicker
    m_cssFile = KStandardDirs::locate("data", "plasma-applet-opendesktop/user.css");
    load(m_cssFile);
    m_cssWatch = new KDirWatch(this);
    m_cssWatch->addFile(m_cssFile);
    connect(m_cssWatch,SIGNAL(dirty(QString)),this,SLOT(load(QString)));
    connect(m_cssWatch,SIGNAL(created(QString)),this,SLOT(load(QString)));
    connect(m_cssWatch,SIGNAL(deleted(QString)),this,SLOT(load(QString)));

    connect(Plasma::Theme::defaultTheme(), SIGNAL(themeChanged()), SLOT(update()));
    connect(KGlobalSettings::self(), SIGNAL(kdisplayPaletteChanged()), SLOT(update()));
}

StyleSheet::~StyleSheet()
{
}

QString StyleSheet::styleSheet() const
{
    return m_styleSheet;
}

void StyleSheet::setFileName(const QString &cssFile)
{
    if (m_cssFile != cssFile) {
        // change watch file watch and load new CSS ...
        m_cssWatch->removeFile(m_cssFile);
        m_cssFile = cssFile;
        load(cssFile);
        m_cssWatch->addFile(m_cssFile);
    }
}

void StyleSheet::setStyleSheet(const QString &css)
{
    m_rawStyleSheet = css;
    update();
}

void StyleSheet::load(const QString &cssFile)
{
    QFile f(this);

    if (cssFile.isEmpty()) {
        f.setFileName(m_cssFile);
    } else {
        f.setFileName(cssFile);
    }
    kDebug() << "(Re)loading CSS" << cssFile;
    if (f.open(QIODevice::ReadOnly)) {
        QTextStream t(&f);
        m_rawStyleSheet = t.readAll();
        f.close();
        update();
    } else {
        kDebug() << "CSS File not loaded, error reading file";
    }
}

void StyleSheet::update()
{
    QPalette p = QPalette();

    QColor text = Plasma::Theme::defaultTheme()->color(Plasma::Theme::TextColor);
    QColor link = Plasma::Theme::defaultTheme()->color(Plasma::Theme::TextColor);
    QColor background = Plasma::Theme::defaultTheme()->color(Plasma::Theme::BackgroundColor);
    link.setAlphaF(qreal(.8));
    QColor linkvisited = Plasma::Theme::defaultTheme()->color(Plasma::Theme::TextColor);
    linkvisited.setAlphaF(qreal(.6));

    m_colors["%textcolor"] = text.name();
    m_colors["%background"] = background.name();
    m_colors["%visitedlink"] = linkvisited.name();
    m_colors["%activatedlink"] = linkvisited.name();
    m_colors["%hoveredlink"] = linkvisited.name();
    m_colors["%link"] = link.name();
    m_colors["%smallfontsize"] = QString("%1pt").arg(KGlobalSettings::smallestReadableFont().pointSize());
    m_colors["%fontsize"] = QString("%1pt").arg(KGlobalSettings::generalFont().pointSize());

    m_styleSheet = m_rawStyleSheet;
    foreach(const QString &k, m_colors.keys()) {
        m_styleSheet.replace(k, m_colors[k]);
    }
    //kDebug() << "CSS:" << m_styleSheet;

    emit styleSheetChanged(m_styleSheet);
}

#include "stylesheet.moc"
