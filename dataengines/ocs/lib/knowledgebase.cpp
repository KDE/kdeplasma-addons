/***************************************************************************
 *   This file is part of KDE.                                             *
 *   Copyright (C) 2009 Marco Martin <notmart@gmail.com>                   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/



#include "knowledgebase.h"

using namespace Attica;

class KnowledgeBase::Private : public QSharedData {
    public:
        QString m_id;
        int m_contentId;
        QString m_user;
        QString m_status;
        QDateTime m_changed;
        QString m_name;
        QString m_description;
        QString m_answer;
        int m_comments;
        KUrl m_detailPage;

        QMap<QString,QString> m_extendedAttributes;

        Private()
            : m_contentId(0),
              m_comments(0)
        {
        }
};

KnowledgeBase::KnowledgeBase()
    : d(new Private)
{
}


KnowledgeBase::KnowledgeBase(const KnowledgeBase& other)
    : d(other.d)
{
}


KnowledgeBase& KnowledgeBase::operator=(const Attica::KnowledgeBase & other)
{
    d = other.d;
    return *this;
}


KnowledgeBase::~KnowledgeBase()
{
}


void KnowledgeBase::setId(QString id)
{
    d->m_id = id;
}

QString KnowledgeBase::id() const
{
    return d->m_id;
}


void KnowledgeBase::setContentId(int id)
{
    d->m_contentId = id;
}

int KnowledgeBase::contentId() const
{
    return d->m_contentId;
}


void KnowledgeBase::setUser(const QString &user)
{
    d->m_user = user;
}

QString KnowledgeBase::user() const
{
    return d->m_user;
}


void KnowledgeBase::setStatus(const QString status)
{
    d->m_status = status;
}

QString KnowledgeBase::status() const
{
    return d->m_status;
}


void KnowledgeBase::setChanged(const QDateTime &changed)
{
    d->m_changed = changed;
}

QDateTime KnowledgeBase::changed() const
{
    return d->m_changed;
}


void KnowledgeBase::setName(const QString &name)
{
    d->m_name = name;
}

QString KnowledgeBase::name() const
{
    return d->m_name;
}


void KnowledgeBase::setDescription(const QString &description)
{
    d->m_description = description;
}

QString KnowledgeBase::description() const
{
    return d->m_description;
}


void KnowledgeBase::setAnswer(const QString &answer)
{
    d->m_answer = answer;
}

QString KnowledgeBase::answer() const
{
    return d->m_answer;
}


void KnowledgeBase::setComments(int comments)
{
    d->m_comments = comments;
}

int KnowledgeBase::comments() const
{
    return d->m_comments;
}


void KnowledgeBase::setDetailPage(const KUrl &detailPage)
{
    d->m_detailPage = detailPage;
}

KUrl KnowledgeBase::detailPage() const
{
    return d->m_detailPage;
}

void KnowledgeBase::addExtendedAttribute( const QString &key, const QString &value )
{
  d->m_extendedAttributes.insert( key, value );
}

QString KnowledgeBase::extendedAttribute( const QString &key ) const
{
  return d->m_extendedAttributes.value( key );
}

QMap<QString,QString> KnowledgeBase::extendedAttributes() const
{
  return d->m_extendedAttributes;
}


bool KnowledgeBase::isValid() const {
  return !(d->m_id.isEmpty());
}
