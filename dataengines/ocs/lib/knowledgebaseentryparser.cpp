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

#include "knowledgebaseentryparser.h"


using namespace Attica;

KnowledgeBaseEntry::Parser::Parser()
{
}

KnowledgeBaseEntry::List KnowledgeBaseEntry::Parser::parseList( const QString &xmlString )
{
  KnowledgeBaseEntry::List KnowledgeBaseList;

  QXmlStreamReader xml( xmlString );

  m_lastMetadata = parseMetadata(xml);

  while ( !xml.atEnd() ) {
    xml.readNext();

    if ( xml.isStartElement() && xml.name() == "content" ) {
      KnowledgeBaseEntry KnowledgeBase = parseKnowledgeBase( xml );
      KnowledgeBaseList.append( KnowledgeBase );
    }
  }

  return KnowledgeBaseList;
}

KnowledgeBaseEntry KnowledgeBaseEntry::Parser::parse( const QString &xmlString )
{
  KnowledgeBaseEntry knowledgeBase;

  QXmlStreamReader xml( xmlString );

  m_lastMetadata = parseMetadata(xml);

  while ( !xml.atEnd() ) {
    xml.readNext();

    if ( xml.isStartElement() && xml.name() == "knowledgebase" ) {
      knowledgeBase = parseKnowledgeBase( xml );
    }
  }

  return knowledgeBase;
}

KnowledgeBaseEntry::Metadata KnowledgeBaseEntry::Parser::lastMetadata()
{
    return m_lastMetadata;
}

KnowledgeBaseEntry::Metadata KnowledgeBaseEntry::Parser::parseMetadata( QXmlStreamReader &xml )
{
    KnowledgeBaseEntry::Metadata meta;
    meta.status = QString();
    meta.message = QString();
    meta.totalItems = 0;
    meta.itemsPerPage = 0;

    while ( !xml.atEnd() ) {
        xml.readNext();
        if (xml.isStartElement() && xml.name() == "meta") {
            while ( !xml.atEnd() ) {
                xml.readNext();
                if (xml.isEndElement() && xml.name() == "meta") {
                    break;
                } else if (xml.isStartElement()) {
                    if (xml.name() == "status") {
                        meta.status = xml.readElementText();
                    } else if (xml.name() == "message") {
                        meta.message = xml.readElementText();
                    } else if (xml.name() == "totalitems") {
                        meta.totalItems = xml.readElementText().toInt();
                    } else if (xml.name() == "itemsperpage") {
                        meta.itemsPerPage = xml.readElementText().toInt();
                    }
                }
            }
            break;
        }
    }

    return meta;
}

KnowledgeBaseEntry KnowledgeBaseEntry::Parser::parseKnowledgeBase( QXmlStreamReader &xml )
{
  KnowledgeBaseEntry knowledgeBase;

  while ( !xml.atEnd() ) {
    xml.readNext();

    if ( xml.isStartElement() ) {
      if ( xml.name() == "id" ) {
        knowledgeBase.setId( xml.readElementText() );
      } else if ( xml.name() == "status" ) {
        knowledgeBase.setStatus( xml.readElementText() );
      } else if ( xml.name() == "contentId" ) {
        knowledgeBase.setContentId( xml.readElementText().toInt() );
      } else if ( xml.name() == "user" ) {
        knowledgeBase.setUser( xml.readElementText() );
      } else if ( xml.name() == "changed" ) {
        knowledgeBase.setChanged( QDateTime::fromString( xml.readElementText(),
          Qt::ISODate ) );
      } else if ( xml.name() == "description" ) {
        knowledgeBase.setDescription( xml.readElementText() );
      } else if ( xml.name() == "answer" ) {
        knowledgeBase.setAnswer( xml.readElementText() );
      } else if ( xml.name() == "comments" ) {
        knowledgeBase.setComments( xml.readElementText().toInt() );
      } else if ( xml.name() == "detailpage" ) {
        knowledgeBase.setDetailPage( KUrl(xml.readElementText()) );
      } else if ( xml.name() == "contentid" ) {
        knowledgeBase.setContentId( xml.readElementText().toInt() );
      } else if ( xml.name() == "name" ) {
        knowledgeBase.setName( xml.readElementText() );
      } else {
        knowledgeBase.addExtendedAttribute( xml.name().toString(),
          xml.readElementText() );
      }
    }

    if ( xml.isEndElement() && xml.name() == "content" ) break;
  }

  return knowledgeBase;
}
