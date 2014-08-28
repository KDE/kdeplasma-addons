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

#include "notemanager.h"

#include <QMutex>
#include <QDebug>

#include "filesystemnotemanager.h"

NoteManager::NoteManager(QObject* parent): QObject(parent)
{
    m_backend = loadBackend();
    m_activeNoteId = "test";
}

Note* NoteManager::note()
{

    return m_backend->loadNote(m_activeNoteId);
//     if (m_activeNote.isNull()) {
//         m_activeNote = m_backend->loadNote(m_activeNoteId);
        //TODO set ownership to QML
//     }
//     return m_activeNote.data();
}

void NoteManager::deleteNoteResources(const QString &id)
{
    m_backend->deleteNoteResources(id);
}

QSharedPointer< AbstractNoteManager > NoteManager::loadBackend()
{
    static QMutex mutex;
    static QWeakPointer<AbstractNoteManager> s_backend;

    mutex.lock();
    QSharedPointer<AbstractNoteManager> manager = s_backend.toStrongRef();
    if (manager.isNull()) {
        manager.reset(new FileSystemNoteManager);
        s_backend = manager;
    }
    mutex.unlock();
    return manager;
}

#include "notemanager.moc"
