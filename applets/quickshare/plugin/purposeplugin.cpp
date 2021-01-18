/*
    SPDX-FileCopyrightText: 2015 Aleix Pol Gonzalez <aleixpol@blue-systems.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "contenttracker.h"
#include <QBuffer>
#include <QImage>
#include <QPixmap>
#include <QQmlEngine>
#include <QQmlExtensionPlugin>
#include <QVariant>
#include <qqml.h>

class PurposeHelper : public QObject
{
    Q_OBJECT
public:
    Q_INVOKABLE static QByteArray variantToBase64(const QVariant &content)
    {
        switch (content.type()) {
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

    static QByteArray imageToBase64(const QImage &img)
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
        qmlRegisterSingletonType<PurposeHelper>("org.kde.plasma.private.purpose", 1, 0, "PurposeHelper", [](QQmlEngine *, QJSEngine *) -> QObject * {
            return new PurposeHelper;
        });
        qmlRegisterType<ContentTracker>("org.kde.plasma.private.purpose", 1, 0, "ContentTracker");
    }
};

#include "purposeplugin.moc"
