/***************************************************************************
 *   Copyright (C) 2012 Matthias Fuchs <mat69@gmx.net>                     *
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

#include "comicinfo.h"

#include <KConfigGroup>
#include <KGlobalSettings>

#include <QtCore/QDir>

class SavingDir::SavingDirPrivate
{
    public:
        SavingDirPrivate(const KConfigGroup &cfg);

        void init();

        QString getDir() const;
        void setDir(const QString &dir);

    private:
        void load();
        void save();
        bool isValid();

    private:
        KConfigGroup mCfg;
        QString mDir;
};


SavingDir::SavingDirPrivate::SavingDirPrivate(const KConfigGroup &cfg)
  : mCfg(cfg)
{
}


void SavingDir::SavingDirPrivate::init()
{
    load();
    save();
}

QString SavingDir::SavingDirPrivate::getDir() const
{
    return mDir;
}

void SavingDir::SavingDirPrivate::setDir(const QString &dir)
{
    mDir = dir;
    save();
}

void SavingDir::SavingDirPrivate::load()
{
    mDir = mCfg.readEntry("savingDir", QString());
    if (!isValid()) {
        mDir = KGlobalSettings::picturesPath();
    }
    if (!isValid()) {
        mDir = KGlobalSettings::downloadPath();
    }
    if (!isValid()) {
        mDir = QDir::homePath();
    }
}

void SavingDir::SavingDirPrivate::save()
{
     mCfg.writeEntry("savingDir", mDir);
}

bool SavingDir::SavingDirPrivate::isValid()
{
    QDir dir;
    return (!mDir.isEmpty() && dir.exists(mDir));
}

SavingDir::SavingDir(const KConfigGroup &cfg)
  : d(new SavingDirPrivate(cfg))
{
    d->init();
}

SavingDir::~SavingDir()
{
    delete d;
}

QString SavingDir::getDir() const
{
    return d->getDir();
}

void SavingDir::setDir(const QString &dir)
{
    d->setDir(dir);
}
