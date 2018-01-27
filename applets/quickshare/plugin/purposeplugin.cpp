/*
    Copyright (C) 2015 Aleix Pol Gonzalez <aleixpol@blue-systems.com>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#include <QVariant>
#include <QQmlEngine>
#include <QQmlExtensionPlugin>
#include <QImage>
#include <QPixmap>
#include <QBuffer>
#include <qqml.h>
#include "contenttracker.h"

class PurposeHelper : public QObject
{
Q_OBJECT
public:
    Q_INVOKABLE static QByteArray variantToBase64(const QVariant& content)
    {
        switch (content.type())
        {
            case QVariant::Image:
                return imageToBase64(content.value<QImage>());
            case QVariant::Pixmap:
                return imageToBase64(content.value<QPixmap>().toImage());
            case QVariant::ByteArray:
                return content.toByteArray().toBase64();
            case QVariant::String:
            default:
                return content.toString().toLatin1().toBase64();
        }
    }

    static QByteArray imageToBase64(const QImage& img)
    {
        QByteArray bytes;
        {
            QBuffer buffer(&bytes);
            buffer.open(QIODevice::WriteOnly);
            bool b = img.save(&buffer, "PNG");
            Q_ASSERT(b);
        }
        return bytes.toBase64();
    }
};

class NotesPlugin : public QQmlExtensionPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QQmlExtensionInterface")
public:
    void registerTypes(const char *uri) override
    {
        Q_UNUSED(uri);
        qmlRegisterSingletonType<PurposeHelper>("org.kde.plasma.private.purpose", 1, 0, "PurposeHelper", [](QQmlEngine*, QJSEngine*) -> QObject* { return new PurposeHelper; });
        qmlRegisterType<ContentTracker>("org.kde.plasma.private.purpose", 1, 0, "ContentTracker");
    }
};

#include "purposeplugin.moc"
