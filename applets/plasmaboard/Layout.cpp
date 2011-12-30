/***************************************************************************
 *   Copyright (C) 2010 by Björn Ruberg <bjoern@ruberg-wegener.de>         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

#include "Layout.h"
#include <QFile>
#include <QXmlStreamReader>
#include <KLocale>

Layout::Layout(QString path)
{
    m_path = path;

    QFile* file = new QFile(path);
    file->open(QIODevice::ReadOnly | QIODevice::Text);
    QXmlStreamReader* xmlReader = new QXmlStreamReader(file);

    // reading in header information
    if (xmlReader->readNextStartElement()) {
        if (xmlReader->name() == "keyboard") {
            m_name = i18n(xmlReader->attributes().value("title").toString().toAscii());
            m_description = i18n(xmlReader->attributes().value("description").toString().toAscii());
        }
    }

    delete file;
    delete xmlReader;
}

QString Layout::description() const
{
    return m_description;
}


QString Layout::name() const
{
    return m_name;
}


QString Layout::path() const
{
    return m_path;
}
