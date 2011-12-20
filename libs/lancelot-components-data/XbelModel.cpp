/*
 *   Copyright (C) 2007, 2008, 2009, 2010 Ivan Cukic <ivan.cukic(at)kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Lesser/Library General Public License version 2,
 *   or (at your option) any later version, as published by the Free
 *   Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU Lesser/Library General Public License for more details
 *
 *   You should have received a copy of the GNU Lesser/Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "XbelModel.h"
#include <KIcon>
#include <QFile>

namespace Lancelot {
namespace Models {

class XbelModel::Private {
public:
    Private(XbelModel * parent)
        : q(parent)
    {}

    void readXbel();
    void readFolder();
    void readBookmark();

    QString filePath;
    QXmlStreamReader xmlReader;

    XbelModel * const q;

};

XbelModel::XbelModel(QString filePath)
    : BaseModel(true), d(new Private(this))
{
    d->filePath = filePath;
    load();
}

XbelModel::~XbelModel()
{
    delete d;
}

void XbelModel::reload()
{
    clear();
    load();
}

void XbelModel::load()
{
    QFile file(d->filePath);

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return;
    }
    d->xmlReader.setDevice(& file);

    while (!d->xmlReader.atEnd()) {
        d->xmlReader.readNext();

        if (d->xmlReader.isStartElement()) {
            if (d->xmlReader.name() == "xbel") {
                d->readXbel();
            }
        }
    }
}

void XbelModel::Private::readXbel()
{
    while (!xmlReader.atEnd()) {
        xmlReader.readNext();

        if (xmlReader.isEndElement() &&
                xmlReader.name() == "xbel")
            break;

        if (xmlReader.isStartElement()) {
            if (xmlReader.name() == "folder")
                readFolder();
            else if (xmlReader.name() == "bookmark")
                readBookmark();
        }
    }
}

void XbelModel::Private::readFolder()
{
    while (!xmlReader.atEnd()) {
        xmlReader.readNext();

        if (xmlReader.isEndElement() && xmlReader.name() == "folder") {
            break;
        }
    }
}

void XbelModel::Private::readBookmark()
{
    Item bookmarkItem;
    bool showBookmark = true;

    KUrl url(xmlReader.attributes().value("href").toString());
    bookmarkItem.data = url.url();
    if (url.isLocalFile()) {
        bookmarkItem.description = url.path();
    } else {
        bookmarkItem.description = url.url();
    }

    while (!xmlReader.atEnd()) {
        xmlReader.readNext();

        if (xmlReader.isEndElement() && xmlReader.name() == "bookmark") {
            break;
        }

        if (xmlReader.name() == "title") {
            bookmarkItem.title = xmlReader.readElementText();

        } else if (xmlReader.name() == "icon") {
            QString icon = xmlReader.attributes().value("name").toString();
            if (!icon.isEmpty()) {
                bookmarkItem.icon = KIcon(icon);
            }

        } else if (xmlReader.name() == "IsHidden") {
            if (xmlReader.readElementText() != "false") {
                showBookmark = false;
            }

        } else if (xmlReader.name() == "isSystemItem") {
            // This is a bit dirty, but needed for a translation bug fix...

            if (xmlReader.readElementText() == "true") {
                bookmarkItem.title = i18n(bookmarkItem.title.toUtf8().data());
            }

        }
    }

    if (showBookmark) {
        q->add(bookmarkItem);
    }
}

} // namespace Models
} // namespace Lancelot

#include "XbelModel.moc"
