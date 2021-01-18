/*
    SPDX-FileCopyrightText: 2014 David Edmundson <davidedmundson@kde.org>
    SPDX-FileCopyrightText: 2015 Kai Uwe Broulik <kde@privat.broulik.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "notesplugin.h"
#include "documenthandler.h"
#include "note.h"
#include "notemanager.h"

// Qt
#include <QFile>

class NotesHelper : public QObject
{
    Q_OBJECT

public:
    explicit NotesHelper(QObject *parent = nullptr)
        : QObject(parent)
    {
    }

    ~NotesHelper() override = default;

    Q_INVOKABLE QString fileContents(const QString &path) const
    {
        const QUrl &url = QUrl::fromUserInput(path);
        if (!url.isValid()) {
            return QString();
        }

        QFile file(url.toLocalFile());
        if (!file.open(QIODevice::ReadOnly)) {
            return QString();
        }

        return QString::fromUtf8(file.readAll());
    }
};

static QObject *notesHelper_provider(QQmlEngine *, QJSEngine *)
{
    return new NotesHelper();
}

void NotesPlugin::registerTypes(const char *uri)
{
    Q_ASSERT(QLatin1String(uri) == QLatin1String("org.kde.plasma.private.notes"));
    qmlRegisterType<DocumentHandler>(uri, 0, 1, "DocumentHandler");
    qmlRegisterType<NoteManager>(uri, 0, 1, "NoteManager");
    qmlRegisterUncreatableType<Note>(uri, 0, 1, "Note", QStringLiteral("Create through NoteManager"));
    qmlRegisterSingletonType<NotesHelper>(uri, 0, 1, "NotesHelper", notesHelper_provider);
}

#include "notesplugin.moc"
