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

#include <QDir>
#include <KUrl>

#include "picture.h"
#include "slideshow.h"
#include <stdlib.h>

SlideShow::SlideShow()
{
	m_filters << "*.jpeg" << "*.jpg" << "*.png" << "*.svg" << "*.svgz"; // use mime types?
	m_slideNumber = 0;
	useRandom = false;
}

SlideShow::~SlideShow()
{
}

void SlideShow::setRandom(bool tmp)
{
	useRandom = tmp;
}

void SlideShow::setDirs(const QStringList &slideShowPath, bool recursive)
{
    m_pictures.clear();
    foreach (const QString &path, slideShowPath) {
        if (recursive) {
            addRecursiveDir(path);
        } else {
            addDir(path);
        }
    }
}

void SlideShow::setImage(const QString &imagePath) 
{
	m_pictures.clear();
	addImage(imagePath);
}

void SlideShow::addImage(const QString &imagePath)
{
	if (!m_pictures.contains(imagePath)) {
		m_pictures.append(imagePath);
	}
}

void SlideShow::addDir(const QString &path)
{
	QDir dir(path);

	dir.setNameFilters(m_filters);
	foreach (const QString &imageFile, dir.entryList(QDir::Files)) {
		addImage(path + "/" + imageFile);
	}
}

void SlideShow::addRecursiveDir(const QString &path)
{
    addDir(path);
    QDir dir(path);

    foreach (const QString &subDir, dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot)) {
        addRecursiveDir(path + "/" + subDir);
    }
}

QImage SlideShow::getImage() 
{
	KUrl url = getUrl();
	Picture myPicture;
	return myPicture.setPicture(url);
}

KUrl SlideShow::getUrl() 
{
	if (!m_pictures.isEmpty()) {

		int index = -1;

		if(useRandom) {
			index = floor((double(rand()) / double(RAND_MAX)) * double(m_pictures.count()));
			kDebug() << "Random was selected and the index was: " << index << " out of " << m_pictures.count() << " images" << endl;
		} else {
			index = m_slideNumber++ % m_pictures.count();
                }

		return KUrl(m_pictures.at(index));
	} else {
		return KUrl("Default");
	}
}
