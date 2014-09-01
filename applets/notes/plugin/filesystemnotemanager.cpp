/*
 * <one line to give the library's name and an idea of what it does.>
 * Copyright (C) 2014  David Edmundson <david@davidedmundson.co.uk>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#include "filesystemnotemanager.h"

#include "note.h"

#include <QUuid>
#include <QDebug>

#include <KDirWatch>

class FileNote : public Note
{
Q_OBJECT
public:
    FileNote(const QString &path, const QString &id);
    void load();
    void save(const QString &text);
private:
    void fileSystemChanged(const QString &path);
    QString m_path;
//     QFileSystemWatcher *m_watcher;
};

FileSystemNoteManager::FileSystemNoteManager()
{
//     n_notesDir = QStandardPaths::writableLocation(QStandardPaths::DataLocation) + "";
    m_notesDir = "/home/david/notes";
}


QStringList FileSystemNoteManager::allNoteIds()
{
    return m_notesDir.entryList(QStringList() << "*.txt");
}

void FileSystemNoteManager::deleteNoteResources(const QString &id)
{
    m_notesDir.remove(id);
}

Note* FileSystemNoteManager::loadNote(const QString &id)
{
    QString idToUse = id;
    if (id.isEmpty()) {
        idToUse = QUuid::createUuid().toString();
    }

    FileNote* note = new FileNote(m_notesDir.absoluteFilePath(idToUse), idToUse);
    return note;
}

FileNote::FileNote(const QString& path, const QString& id):
    Note(id),
    m_path(path)
{
    //FIXME Aleix said this was bad... not sure why, I'm checking the path..

    //OPTIMISATION right now every time we save a note, we're going to read it back in again
    //not a huge problem as we don't save that often
    //blocking signals isn't enough as KDirWatch is a bit slow and we finish saving before it says we changed a file
    KDirWatch::self()->addFile(path);

    connect(KDirWatch::self(), &KDirWatch::created, this, &FileNote::fileSystemChanged);
    connect(KDirWatch::self(), &KDirWatch::dirty, this, &FileNote::fileSystemChanged);

    load();
}

void FileNote::load()
{
    QFile file(m_path);
    if (file.open(QIODevice::QIODevice::ReadOnly | QIODevice::Text)) {
        setNoteText(file.readAll());
    }
}

void FileNote::save(const QString &text)
{
    if (text == noteText()) {
        return;
    }

    QFile file(m_path);
    if (file.open(QIODevice::QIODevice::WriteOnly | QIODevice::Text)) {
        file.write(text.toLatin1());
    } else {
        qWarning() << "could not write notes to file " << m_path;
    }
    setNoteText(text);
}

void FileNote::fileSystemChanged(const QString &path)
{
    if (path == m_path) {
        load();
    }
}



#include "filesystemnotemanager.moc"
