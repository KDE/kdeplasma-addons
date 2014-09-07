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

#ifndef NOTE_H
#define NOTE_H

#include <QObject>

class Note : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString id READ id CONSTANT)
    Q_PROPERTY(QString noteText READ noteText NOTIFY noteTextChanged)

public:
    explicit Note(const QString &id);
    QString id() const;

    //what's in the plasmoid
    //backends save this and write into storedText
    QString noteText() const ;
    void setNoteText(const QString &text);

public Q_SLOTS:
    virtual void save(const QString &text) = 0;

    //FUTURE
//     status  None, Ready, Loading, Error

Q_SIGNALS:
    void noteTextChanged();
private:
    const QString m_id;
    QString m_noteText;
};

#endif // NOTE_H
