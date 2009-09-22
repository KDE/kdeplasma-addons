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

#ifndef ATTICA_KNOWLEDGEBASE_H
#define ATTICA_KNOWLEDGEBASE_H

#include "atticaclient_export.h"

#include <QtCore/QDateTime>
#include <QtCore/QSharedDataPointer>

#include <KUrl>


namespace Attica
{

class ATTICA_EXPORT KnowledgeBase
{
  public:
    typedef QList<KnowledgeBase> List;
    class Parser;
    struct Metadata
    {
        QString status;
        QString message;
        int totalItems;
        int itemsPerPage;
    };

    KnowledgeBase();
    KnowledgeBase(const KnowledgeBase& other);
    KnowledgeBase& operator=(const KnowledgeBase& other);
    ~KnowledgeBase();

    void setId(QString id);
    QString id() const;

    void setContentId(int id);
    int contentId() const;

    void setUser(const QString &user);
    QString user() const;

    void setStatus(const QString status);
    QString status() const;

    void setChanged(const QDateTime &changed);
    QDateTime changed() const;

    void setName(const QString &name);
    QString name() const;

    void setDescription(const QString &description);
    QString description() const;

    void setAnswer(const QString &answer);
    QString answer() const;

    void setComments(int comments);
    int comments() const;

    void setDetailPage(const KUrl &detailPage);
    KUrl detailPage() const;

    void addExtendedAttribute( const QString &key, const QString &value );
    QString extendedAttribute( const QString &key ) const;

    QMap<QString,QString> extendedAttributes() const;

    bool isValid() const;

  private:
    class Private;
    QSharedDataPointer<Private> d;
};

}

#endif

