/*
    This file is part of KDE.

    Copyright (c) 2008 Cornelius Schumacher <schumacher@kde.org>
    Copyright (c) 2009 Marco Martin <notmart@gmail.com>

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
#ifndef ATTICA_KNOWLEDGEBASEENTRYPARSER_H
#define ATTICA_KNOWLEDGEBASEENTRYPARSER_H

// WARNING: QXmlStreamReader cannot be forward declared (Qt 4.5)
#include <QtXml/QXmlStreamReader>

#include "knowledgebaseentry.h"


namespace Attica {

class KnowledgeBaseEntry::Parser
{
  public:
    Parser();

    KnowledgeBaseEntry parse( const QString &xml );
    KnowledgeBaseEntry::List parseList( const QString &xml );
    KnowledgeBaseEntry::Metadata lastMetadata();

  protected:
    KnowledgeBaseEntry parseKnowledgeBase( QXmlStreamReader &xml );
    KnowledgeBaseEntry::Metadata parseMetadata( QXmlStreamReader &xml );

  private:
    KnowledgeBaseEntry::Metadata m_lastMetadata;
};

}

#endif
