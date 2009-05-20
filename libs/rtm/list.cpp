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

#include "list.h"

namespace RTM {

class ListPrivate {
  ListPrivate(List *parent)
    : q(parent)
  {}
  
  friend class List;
  List *q;
  
  QString name;
  RTM::ListId listId;
  bool smart;
  QString filter;

  // none of the following are used yet. However, they exsist in the
  //  list example and so I thought that they should be included.
  bool deleted;
  bool locked;
  bool archived;
  int position;
};

List::List(Session* session) 
  : QObject(session),
  d(new ListPrivate(this))
{ }

List::~List()
{
  delete d;
}

QString List::name() const { 
  return d->name;
}

ListId List::id() const { 
  return d->listId;
}

bool List::isSmart() const { 
  return d->smart; 
}

QString List::filter() const { 
  return d->filter; 
}

void List::setName(const QString& name) { 
  d->name = name; 
}

void List::setId(qulonglong id) {
  d->listId = id; 
}

void List::setSmart(bool smart) { 
  d->smart = smart; 
}

void List::setFilter(const QString& filter) { 
  d->filter = filter; 
}

}
#include "list.moc"
