/*
 *   Copyright (C) 2007 Ivan Cukic <ivan.cukic+kde@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License version 2,
 *   or (at your option) any later version, as published by the Free
 *   Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "XbelModel.h"
#include <KIcon>
#include <QFile>

namespace Models {

XbelModel::XbelModel(QString filePath)
    : BaseModel(true), m_filePath(filePath)
{
    load();
}

XbelModel::~XbelModel()
{
}

void XbelModel::reload()
{
    clear();
    load();
}

void XbelModel::load()
{
    QFile file(m_filePath);

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return;
    }
    m_xmlReader.setDevice(& file);

    while (!m_xmlReader.atEnd()) {
        m_xmlReader.readNext();

        if (m_xmlReader.isStartElement()) {
            if (m_xmlReader.name() == "xbel") {
                readXbel();
            }
        }
    }
}

void XbelModel::readXbel()
{
    while (!m_xmlReader.atEnd()) {
        m_xmlReader.readNext();

        if (m_xmlReader.isEndElement() &&
                m_xmlReader.name() == "xbel")
            break;

        if (m_xmlReader.isStartElement()) {
            if (m_xmlReader.name() == "folder")
                readFolder();
            else if (m_xmlReader.name() == "bookmark")
                readBookmark();
        }
    }
}

void XbelModel::readFolder()
{
    while (!m_xmlReader.atEnd()) {
        m_xmlReader.readNext();

        if (m_xmlReader.isEndElement() && m_xmlReader.name() == "folder") {
            break;
        }
    }
}

void XbelModel::readBookmark()
{
    Item bookmarkItem;
    bool showBookmark = true;

    KUrl url(m_xmlReader.attributes().value("href").toString());
    bookmarkItem.data = url.url();
    if (url.isLocalFile()) {
        bookmarkItem.description = url.path();
    } else {
        bookmarkItem.description = url.url();
    }

    while (!m_xmlReader.atEnd()) {
        m_xmlReader.readNext();

        if (m_xmlReader.isEndElement() && m_xmlReader.name() == "bookmark") {
            break;
        }

        if (m_xmlReader.name() == "title") {
            bookmarkItem.title = m_xmlReader.readElementText();
        } else if (m_xmlReader.name() == "icon") {
            QString icon = m_xmlReader.attributes().value("name").toString();
            if (icon != QString()) {
                bookmarkItem.icon = KIcon(icon);
            }
        } else if (m_xmlReader.name() == "IsHidden") {
            if (m_xmlReader.readElementText() != "false") {
                showBookmark = false;
            }
        }
    }

    if (showBookmark) {
        add(bookmarkItem);
    }
}

} // namespace Models

#include "XbelModel.moc"
