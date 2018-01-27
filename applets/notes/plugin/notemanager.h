/*
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

#ifndef NOTEMANAGER_H
#define NOTEMANAGER_H

#include <QObject>
#include <QSharedPointer>
#include <QPointer>

#include "abstractnoteloader.h"
class Note;

class NoteManager : public QObject
{
    Q_OBJECT

public:
    explicit NoteManager(QObject* parent = nullptr);

    /**
     * Loads the note for the ID given
     * Ownership is passed to the QML context
     */
    Q_INVOKABLE Note* loadNote(const QString &id);

    /**
     * Remove any resources associated with the note ID
     */
    Q_INVOKABLE void deleteNoteResources(const QString &id);

    //LATER QAbstractListModel* notesModel(); //list of all notes

private:
    //ref count backends so that we only have for all notes
    static QSharedPointer<AbstractNoteLoader> loadBackend();

    QSharedPointer<AbstractNoteLoader> m_backend;
    QWeakPointer<Note> m_lastNote;
};






#endif // NOTEMANAGER_H
