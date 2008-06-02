/***************************************************************************
 *   Copyright  2008 by Thomas Coopman <thomas.coopman@gmail.com>          *
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

#ifndef SLIDESHOW_H_
#define SLIDESHOW_H_

#include <QString>
#include <QStringList>
#include <QImage>
#include <math.h>
#include <iostream>

class KUrl;

class SlideShow
{
	public:
		SlideShow();
		~SlideShow();
		
		void setDirs(const QStringList &slideShowPaths, bool recursive = false);
		void setImage(const QString &imagePath);
		void setRandom(bool);
		QImage getImage();
		
	private:
		QStringList m_pictures;
		QStringList m_filters;
		int m_slideNumber;
		bool useRandom;
		
		void addImage(const QString &imagePath);
		void addDir(const QString &path);
		void addRecursiveDir(const QString &path);
		KUrl getUrl();
		QList<int> indexList;
		int randomInt;

};

#endif /*SLIDESHOW_H_*/
