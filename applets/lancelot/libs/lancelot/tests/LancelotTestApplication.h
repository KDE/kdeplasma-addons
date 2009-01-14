/*
 *   Copyright (C) 2007 Ivan Cukic <ivan.cukic+kde@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Lesser/Library General Public License version 2,
 *   or (at your option) any later version, as published by the Free
 *   Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU Lesser/Library General Public License for more details
 *
 *   You should have received a copy of the GNU Lesser/Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef LANCELOT_TEST_APPLICATION_H
#define LANCELOT_TEST_APPLICATION_H

#include <kuniqueapplication.h>
#include <QTimer>
#include <QSet>
#include <QStringList>

#include "LancelotTestWindow.h"

class LancelotTestApplication: public KUniqueApplication
{
    Q_OBJECT

public:
    static int main(int argc, char **argv);

protected:
    LancelotTestWindow * window;

    static LancelotTestApplication * m_application;

    LancelotTestApplication(int argc, char **argv);
    void init();

    virtual ~LancelotTestApplication();

};

#endif /*LANCELOT_TEST_APPLICATION_H*/
