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

#ifndef GROUPINFO_H
#define GROUPINFO_H

#include <QtCore/QString>
#include <QtCore/QSet>

#include <Plasma/Plasma>

class GroupInfoPrivate;

class GroupInfo
{
    public:
        explicit GroupInfo(const QString &name, const QString &prettyName = QString());
        GroupInfo(const GroupInfo &other);
        ~GroupInfo();
        void setFormFactors(QSet<Plasma::FormFactor> formFactors);
        void setIcon(const QString &icon);

        QString name() const;
        QString prettyName() const;
        QSet<Plasma::FormFactor> formFactors() const;
        QString icon() const;

        GroupInfo &operator=(const GroupInfo &gi);
        bool operator==(const GroupInfo &gi) const;
        bool operator<(const GroupInfo &gi) const;

    private:
        GroupInfoPrivate *const d;
};

#endif //GROUPINFO_H
