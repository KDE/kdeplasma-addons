/*
    This file is part of KDE.

    Copyright (c) 2008 Cornelius Schumacher <schumacher@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,
    USA.
*/

#include "folder.h"

using namespace Attica;

class Folder::Private : public QSharedData {
    public:
        QString m_id;  
        QString m_name;
        int m_messageCount;
        QString m_type;

        Private()
            : m_messageCount(0)
        {
        }
};

Folder::Folder()
  : d(new Private)
{
}

Folder::Folder(const Folder& other)
    : d(other.d)
{
}

Folder& Folder::operator=(const Folder& other)
{
    d = other.d;
    return *this;
}

Folder::~Folder()
{
}


void Folder::setId( const QString &u )
{
  d->m_id = u;
}

QString Folder::id() const
{
  return d->m_id;
}

void Folder::setName( const QString &name )
{
  d->m_name = name;
}

QString Folder::name() const
{
  return d->m_name;
}

void Folder::setMessageCount( int c )
{
  d->m_messageCount = c;
}

int Folder::messageCount() const
{
  return d->m_messageCount;
}

void Folder::setType( const QString &v )
{
  d->m_type = v;
}

QString Folder::type() const
{
  return d->m_type;
}


bool Folder::isValid() const {
  return !(d->m_id.isEmpty());
}
