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

#include "abstractnotemanager.h"
class Note;

class NoteManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(Note* activeNote READ note NOTIFY idChanged);
//     Q_PROPERTY(QString noteId READ noteId WRITE noteId NOTIFY idChanged);
public:
    explicit NoteManager(QObject* parent = 0);

    /**
     * Load a note with the given ID
     * A blank ID will create a new note
     * Ownership is transferred to the QML engine
     */
    Note* note();

    /**
     * Remove any resources associated with the note ID
     */
    Q_INVOKABLE void deleteNoteResources(const QString &id);

    //LATER QAbstractListModel* notesModel(); //list of all notes

Q_SIGNALS:
    void idChanged();

private:
    //ref count backends so that we only have for all notes
    static QSharedPointer<AbstractNoteManager> loadBackend();

    QSharedPointer<AbstractNoteManager> m_backend;
    QString m_activeNoteId;
    QPointer<Note> m_activeNote;
};






#endif // NOTEMANAGER_H
