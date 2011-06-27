/*
 *   Copyright 2009 Andrew Stromme <astromme@chatonka.com>
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

#ifndef LISTSSOURCE_H
#define LISTSSOURCE_H

#include <rtm/rtm.h>

#include <QTimer>

#include <Plasma/DataContainer>
#include <Plasma/Service>
#include <Plasma/ServiceJob>

#include "listsource.h"

class RtmEngine;

class ListsSource : public Plasma::DataContainer
{
Q_OBJECT

public:
    ListsSource(RtmEngine* engine, RTM::Session *session);
    ~ListsSource();

    ListSource* setupListSource(const QString& source);

public slots:
    void refresh();

private slots:
    void listsChanged();
    void listChanged(RTM::List*);
    void loadCache();

private:
    RtmEngine *m_engine;
    RTM::Session *m_session;
    QTimer timer;
};

#endif
