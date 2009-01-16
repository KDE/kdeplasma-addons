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

#ifndef PREVIEWITEMMODEL_H
#define PREVIEWITEMMODEL_H

#include <QAbstractListModel>
#include <QList>
#include <QUrl>

class PreviewItemModel : public QAbstractListModel
{
   Q_OBJECT

   public:
      explicit PreviewItemModel(const QList<QUrl> &urls, QObject *parent = 0)
         : QAbstractListModel(parent), urlList(urls) {}
      
      int rowCount(const QModelIndex &parent = QModelIndex()) const;
      QVariant data(const QModelIndex &index, int role) const;
      void setUrlList(const QList<QUrl> &urls);
      void addUrl(const QUrl &url);

   private:
      QList<QUrl> urlList;
};
#endif
