/*
 *   Copyright (C) 2008 Christian Weilbach <dunsens@web.de>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License version 2 as
 *   published by the Free Software Foundation
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

// Here we avoid loading the header multiple times
#ifndef incomingmsg_HEADER
#define incomingmsg_HEADER

#include <QtDBus/QDBusConnection>

// We need the Plasma Applet headers
#include <Plasma/Applet>
#include <Plasma/Svg>

#include "ui_widget.h"

class QGraphicsLinearLayout;

namespace Plasma
{
class Label;
class PushButton;
class TextEdit;
}

// Define our plasma Applet
class IncomingMsg : public Plasma::Applet
{
    Q_OBJECT
public:
    // Basic Create/Destroy
    IncomingMsg(QObject *parent, const QVariantList &args);
    ~IncomingMsg();
    void init();

protected:
    void createConfigurationInterface(KConfigDialog *parent);
    void constraintsEvent(Plasma::Constraints);

    void initEvolutionLayout();
    void initKMailLayout();
    void initXChatLayout();
    void initKopeteLayout();
    void initPidginLayout();

protected slots:
    void configAccepted();

private:
    void clearLayout();
    void initLayout();

    // text labels
    Plasma::Label *mEvolutionLabel, *mEvolutionIconLabel,
        *mKMailLabel, *mKMailIconLabel, *mXChatLabel,
        *mXChatIconLabel, *mKopeteLabel, *mKopeteIconLabel,
        *mPidginLabel, *mPidginIconLabel, *mErrorLabel;

    QGraphicsLinearLayout *mLayout, *mEvolutionLayout,
        *mKMailLayout, *mXChatLayout, *mKopeteLayout,
        *mPidginLayout;

    bool mShowKMail, mShowXChat, mShowKopete, mShowPidgin;

    Ui::incomingmsgConfig ui;

private slots:
    void slotNewEvolutionMail();
    void slotNewKMailMail();
    void slotNewPidginIM();
    void slotNewKopeteIM(const QString&);
    void slotNewXChatIM();
};

// This is the command that links your applet to the .desktop file
K_EXPORT_PLASMA_APPLET(incomingmsg, IncomingMsg)
#endif


void verboseLog(QString);
