/*
 * Copyright 2008  Petri Damsten <damu@iki.fi>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "appkey.h"
#include <KWindowSystem>

AppKey::AppKey(QWidget *parent)
    : KDialog(parent)
{
    setupUi(mainWidget());
    setButtons(KDialog::Ok | KDialog::Cancel);
    pasteButton->setClearButtonShown(false);
    connect(appButton, SIGNAL(clicked()), this, SLOT(appClicked()));
    connect(pasteButton, SIGNAL(keySequenceChanged(QKeySequence)),
            this, SLOT(enableWidgets()));
    enableWidgets();
}

AppKey::~AppKey()
{
}

void AppKey::appClicked()
{
    appButton->setText(i18nc("Button to select an application by clicking on its window",
                             "Click application"));
    appButton->setIcon(KIcon());
    connect(KWindowSystem::self(), SIGNAL(activeWindowChanged(WId)),
            this, SLOT(activeWindowChanged(WId)));
}

void AppKey::enableWidgets()
{
    enableButtonOk(!appButton->text().isEmpty() && !pasteButton->keySequence().isEmpty());
}

void AppKey::activeWindowChanged(WId id)
{
    KWindowInfo info = KWindowSystem::windowInfo(id, 0, NET::WM2WindowClass);
    appButton->setText(info.windowClassClass());
    appButton->setIcon(KIcon(info.windowClassClass().toLower()));
    app = info.windowClassClass();
    appButton->setChecked(false);
    disconnect(KWindowSystem::self(), SIGNAL(activeWindowChanged(WId)),
               this, SLOT(activeWindowChanged(WId)));
    enableWidgets();
}

#include "appkey.moc"
