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

#ifndef RTM_H
#define RTM_H

#include "rtm_export.h"

#include <QString>

namespace RTM {
  enum State { Mutable, Hashed, RequestSent, RequestReceived };
  enum Permissions { None , Read, Write, Delete };
  const QString baseAuthUrl = "https://www.rememberthemilk.com/services/auth/?";
  const QString baseMethodUrl = "https://api.rememberthemilk.com/services/rest/?";

  /** Timlines are unsigned longs that map to a "session" in which
    actions are undoable */
  typedef unsigned long Timeline;

  typedef qulonglong ListId;
  typedef qulonglong TaskId;
  typedef qulonglong NoteId;
  typedef qulonglong TaskSeriesId;
  typedef qulonglong ListSeriesId;
  typedef qulonglong LocationId;
  typedef QString Tag;

  class List;
  class Request;
  class Auth;
  class Task;
  class Note;


  class Location
  {
  };
  class Contact
  {
  };
  class Group
  {
  };
}


#endif
