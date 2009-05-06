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

#include "LancelotTestApplication.h"

#include <kcmdlineargs.h>
#include <kaboutdata.h>
#include <QIcon>

#include <KAction>
#include <KStandardAction>

#include <lancelot/Global.h>
#include "LancelotTestWindow.h"

LancelotTestApplication * LancelotTestApplication::m_application = NULL;

LancelotTestApplication::LancelotTestApplication(int argc, char **argv)
    : KUniqueApplication(argc, argv), window(0)
{
    init();
}

void LancelotTestApplication::init()
{
    window = new LancelotTestWindow();
    window->show();
}

LancelotTestApplication::~LancelotTestApplication()
{
}

int LancelotTestApplication::main(int argc, char **argv)
{
    KCmdLineArgs::init(argc, argv,
        new KAboutData(
            QByteArray("lancelot-test"),
            QByteArray(""),
            ki18n("Lancelot"),
            QByteArray("0.2"),
            ki18n("Next generation application launcher"),
            KAboutData::License_GPL,
            ki18n("(C) 2007 Ivan Čukić"),
            KLocalizedString(),
            QByteArray(""),
            QByteArray("ivan(dot)cukic(at)kdemail(dot)com")
        )
    );

    LancelotTestApplication::m_application = new LancelotTestApplication(argc, argv);
    //Lancelot::Instance::setHasApplication(true);

    return LancelotTestApplication::m_application->exec();
}


