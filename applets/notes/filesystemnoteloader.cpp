/*
 * SPDX-FileCopyrightText: 2014 David Edmundson <david@davidedmundson.co.uk>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 */

#include "filesystemnoteloader.h"

#include "note.h"

#include <QDebug>
#include <QStandardPaths>
#include <QUuid>

#include <KDirWatch>
#include <KIO/CopyJob>

class FileNote : public Note
{
    Q_OBJECT
public:
    FileNote(const QString &path, const QString &id);
    void load();
    void save(const QString &text) override;

private:
    void fileSystemChanged(const QString &path);
    QString m_path;
    KDirWatch *m_watcher;
};

FileSystemNoteLoader::FileSystemNoteLoader()
{
    const QString genericDataLocation = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation);
    const QString suffix = QStringLiteral("plasma_notes");
    QDir(genericDataLocation).mkdir(suffix);
    m_notesDir.setPath(genericDataLocation + QDir::separator() + suffix);
}

QStringList FileSystemNoteLoader::allNoteIds()
{
    return m_notesDir.entryList(QStringList{QStringLiteral("*.txt")});
}

void FileSystemNoteLoader::deleteNoteResources(const QString &id)
{
    const QUrl url = QUrl::fromLocalFile(m_notesDir.absoluteFilePath(id));
    KIO::Job *job = KIO::trash(url);
    if (!job->exec()) {
        m_notesDir.remove(id);
    }
    m_idToApplet.remove(id);
}

Note *FileSystemNoteLoader::loadNote(const QString &id, uint appletId)
{
    QString idToUse = id;

    if (idToUse.isEmpty()) {
        idToUse = QUuid::createUuid().toString().mid(1, 34);
        m_idToApplet.insert(idToUse, appletId);
    } else {
        if (m_idToApplet.contains(id)) {
            uint existingApplet = m_idToApplet.value(id);
            if (existingApplet != appletId) {
                // If two applets are using the same note id, which can
                // happen if e.g. the panel is cloned, we copy the
                // note file to a new id and assign one file per applet.
                // This avoids having different applets using the same
                // file.
                idToUse = QUuid::createUuid().toString().mid(1, 34);
                QFile::copy(m_notesDir.absoluteFilePath(id), m_notesDir.absoluteFilePath(idToUse));
                m_idToApplet.insert(idToUse, appletId);
            }
        } else {
            m_idToApplet.insert(idToUse, appletId);
        }
    }

    return new FileNote(m_notesDir.absoluteFilePath(idToUse), idToUse);
}

FileNote::FileNote(const QString &path, const QString &id)
    : Note(id)
    , m_path(path)
    , m_watcher(new KDirWatch(this))
{
    m_watcher->addFile(path);

    connect(m_watcher, &KDirWatch::created, this, &FileNote::fileSystemChanged);
    connect(m_watcher, &KDirWatch::dirty, this, &FileNote::fileSystemChanged);

    load();
}

void FileNote::load()
{
    QFile file(m_path);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        setNoteText(QString::fromUtf8(file.readAll()));
    }
}

void FileNote::save(const QString &text)
{
    if (text == noteText()) {
        return;
    }

    m_watcher->removeFile(m_path);

    QFile file(m_path);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        file.write(text.toUtf8());
    } else {
        qWarning() << "Could not write notes to file" << m_path;
    }
    setNoteText(text);

    m_watcher->addFile(m_path);
}

void FileNote::fileSystemChanged(const QString &path)
{
    if (path == m_path) {
        load();
    }
}

#include "filesystemnoteloader.moc"
