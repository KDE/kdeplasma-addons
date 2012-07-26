/*
 *   Copyright 2009 Andrew Stromme <astromme@chatonka.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef RTM_NOTE_H
#define RTM_NOTE_H

// Qt Includes
#include <QHash>
#include <QString>

// Local Includes
#include "rtm.h"

typedef QHash<RTM::NoteId, RTM::Note> Notes;

namespace RTM {

class RTM_EXPORT Note {
public:
  Note(RTM::NoteId id, const QString& title, const QString& text)
  {
    noteId = id;
    m_title = title;
    m_text = text;
  }
  
  RTM::NoteId id() const { return noteId; }
  QString title() const { return m_title; }
  QString text() const { return m_text; }

protected:
  RTM::NoteId noteId;
  QString m_text;
  QString m_title;
};

} // rtm namespace

#endif
