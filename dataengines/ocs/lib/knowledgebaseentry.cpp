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



#include "knowledgebaseentry.h"

using namespace Attica;

class KnowledgeBaseEntry::Private : public QSharedData {
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

KnowledgeBaseEntry::KnowledgeBaseEntry()
    : d(new Private)
{
}


KnowledgeBaseEntry::KnowledgeBaseEntry(const KnowledgeBaseEntry& other)
    : d(other.d)
{
}


KnowledgeBaseEntry& KnowledgeBaseEntry::operator=(const Attica::KnowledgeBaseEntry & other)
{
    d = other.d;
    return *this;
}


KnowledgeBaseEntry::~KnowledgeBaseEntry()
{
}


void KnowledgeBaseEntry::setId(QString id)
{
    d->m_id = id;
}

QString KnowledgeBaseEntry::id() const
{
    return d->m_id;
}


void KnowledgeBaseEntry::setContentId(int id)
{
    d->m_contentId = id;
}

int KnowledgeBaseEntry::contentId() const
{
    return d->m_contentId;
}


void KnowledgeBaseEntry::setUser(const QString &user)
{
    d->m_user = user;
}

QString KnowledgeBaseEntry::user() const
{
    return d->m_user;
}


void KnowledgeBaseEntry::setStatus(const QString status)
{
    d->m_status = status;
}

QString KnowledgeBaseEntry::status() const
{
    return d->m_status;
}


void KnowledgeBaseEntry::setChanged(const QDateTime &changed)
{
    d->m_changed = changed;
}

QDateTime KnowledgeBaseEntry::changed() const
{
    return d->m_changed;
}


void KnowledgeBaseEntry::setName(const QString &name)
{
    d->m_name = name;
}

QString KnowledgeBaseEntry::name() const
{
    return d->m_name;
}


void KnowledgeBaseEntry::setDescription(const QString &description)
{
    d->m_description = description;
}

QString KnowledgeBaseEntry::description() const
{
    return d->m_description;
}


void KnowledgeBaseEntry::setAnswer(const QString &answer)
{
    d->m_answer = answer;
}

QString KnowledgeBaseEntry::answer() const
{
    return d->m_answer;
}


void KnowledgeBaseEntry::setComments(int comments)
{
    d->m_comments = comments;
}

int KnowledgeBaseEntry::comments() const
{
    return d->m_comments;
}


void KnowledgeBaseEntry::setDetailPage(const KUrl &detailPage)
{
    d->m_detailPage = detailPage;
}

KUrl KnowledgeBaseEntry::detailPage() const
{
    return d->m_detailPage;
}

void KnowledgeBaseEntry::addExtendedAttribute( const QString &key, const QString &value )
{
  d->m_extendedAttributes.insert( key, value );
}

QString KnowledgeBaseEntry::extendedAttribute( const QString &key ) const
{
  return d->m_extendedAttributes.value( key );
}

QMap<QString,QString> KnowledgeBaseEntry::extendedAttributes() const
{
  return d->m_extendedAttributes;
}


bool KnowledgeBaseEntry::isValid() const {
  return !(d->m_id.isEmpty());
}
