/*
 *  Copyright 2008 by Alessandro Diaferia <alediaferia@gmail.com>

 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License as
 *  published by the Free Software Foundation; either version 2 of 
 *  the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "previewitemmodel.h"
#include <KUrl>
#include <KMimeType>
#include <KDebug>
#include <KIcon>
#include <KFileItem>


int PreviewItemModel::rowCount( const QModelIndex &parent) const
{
   Q_UNUSED(parent);
   return urlList.count();
}

QVariant PreviewItemModel::data(const QModelIndex &index, int role) const
{
   if( !index.isValid() )
      return QVariant();

   if( index.row() >= urlList.count() )
      return QVariant();

   if( role == Qt::DisplayRole )
      return KUrl(urlList.at( index.row() )).fileName();

   else if( role == Qt::UserRole )
      return urlList.at( index.row() );

   else if( role == Qt::DecorationRole ){
      //KMimeType::Ptr mimeType = KMimeType::findByUrl(KUrl(urlList.at( index.row() )), 0, true);
      KFileItem item(KFileItem::Unknown, KFileItem::Unknown, KUrl(urlList.at( index.row() )));
      KIcon icon(item.iconName(), 0, item.overlays());
      return icon;
   }
   else
      return QVariant();
}

void PreviewItemModel::setUrlList(const QList<QUrl> &urls)
{
   urlList = urls;
}

void PreviewItemModel::addUrl(const QUrl &url)
{
   //beginInsertRows(QModelIndex(), urlList.count(), urlList.count()+1);
   urlList.append(url);
   kDebug()<<urlList;
   //endInsertRows();
}


#include "previewitemmodel.moc"
