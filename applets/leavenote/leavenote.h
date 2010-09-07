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
#ifndef LEAVENOTE_H
#define LEAVENOTE_H

#include <QHash>
#include <QTimer>

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
class LeaveNote : public Plasma::Applet
{
    Q_OBJECT
public:
    // Basic Create/Destroy
    LeaveNote(QObject *parent, const QVariantList &args);
    ~LeaveNote();

    void init();
    void paintInterface(QPainter *painter,
                        const QStyleOptionGraphicsItem *option,
                        const QRect& contentsRect);
public slots:
    void configChanged();

protected:
    void constraintsEvent(Plasma::Constraints constraints);
    void createConfigurationInterface(KConfigDialog *parent);

protected slots:
    void configAccepted();

private slots:
    void slotSend();
    void slotLimitMessageLength();
    void slotWaitForKNotes();

private:

    void createNote(const QString& title, const QString& msg);
    bool checkKNotesDBusInterface();
    void incrementMessageCount();

    // text labels
    Plasma::Label *mLabel;
    Plasma::Label *mCountLabel;

    // svg for theme
    Plasma::Svg mTheme;

    // text edit
    Plasma::TextEdit *mTextEdit;
    Plasma::PushButton *mSendButton;

    QGraphicsLinearLayout* mLayout;

    QList<QHash<QString, QVariant> > mMsgCache;
    QTimer *mTimer;

    /* bools for the dbus daemon */
    bool mUseKNotes, mUseNotification;

    Ui::leavenoteConfig ui;

    int mMessageCounter;
};

// This is the command that links your applet to the .desktop file
K_EXPORT_PLASMA_APPLET(leavenote, LeaveNote)
#endif /* LEAVENOTE_H */

