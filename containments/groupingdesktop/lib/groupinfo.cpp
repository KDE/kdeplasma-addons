/*
 *   Copyright 2010 by Giulio Camuffo <giuliocamuffo@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "groupinfo.h"

class GroupInfoPrivate
{
    public:
        GroupInfoPrivate()
        {
            formFactors << Plasma::Planar << Plasma::MediaCenter;
        }

        ~GroupInfoPrivate()
        {}

        QString name;
        QString prettyName;
        QSet<Plasma::FormFactor> formFactors;
        QString icon;
};

GroupInfo::GroupInfo(const QString &name, const QString &prettyName)
          : d(new GroupInfoPrivate())
{
    d->name = name;
    d->prettyName = prettyName;
}

GroupInfo::GroupInfo(const GroupInfo &other)
          : d(new GroupInfoPrivate())
{
    *d = *other.d;
}

GroupInfo::~GroupInfo()
{
    delete d;
}

void GroupInfo::setFormFactors(QSet<Plasma::FormFactor> formFactors)
{
    d->formFactors = formFactors;
}

void GroupInfo::setIcon(const QString &icon)
{
    d->icon = icon;
}

QString GroupInfo::name() const
{
    return d->name;
}

QString GroupInfo::prettyName() const
{
    return d->prettyName;
}

QSet<Plasma::FormFactor> GroupInfo::formFactors() const
{
    return d->formFactors;
}

QString GroupInfo::icon() const
{
    return d->icon;
}

GroupInfo &GroupInfo::operator=(const GroupInfo &gi)
{
    if (this != &gi) {
        *d = *gi.d;
    }
    return *this;
}

bool GroupInfo::operator==(const GroupInfo &gi) const
{
    return (gi.name() == d->name);
}

bool GroupInfo::operator<(const GroupInfo &gi) const
{
    return (gi.name() < d->name);
}
