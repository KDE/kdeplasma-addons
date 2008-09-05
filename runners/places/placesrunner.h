/*
 *   Copyright 2008 David Edmundson <kde@davidedmundson.co.uk>
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

#ifndef PLACESRUNNER_H
#define PLACESRUNNER_H

#include <KGenericFactory>
#include <KIcon>

#include <plasma/abstractrunner.h>
#include <kfileplacesmodel.h>

class PlacesRunner : public Plasma::AbstractRunner
{
    Q_OBJECT

public:
    PlacesRunner(QObject* parent, const QVariantList &args);
    ~PlacesRunner();

    void match(Plasma::RunnerContext &context);
    void run(const Plasma::RunnerContext &context, const Plasma::QueryMatch &action);
private:
    KFilePlacesModel *m_filePlaces;
private slots:
    void setupComplete(QModelIndex, bool);
};

K_EXPORT_PLASMA_RUNNER(bookmarksrunner, PlacesRunner)

#endif
