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

/* Description:
 * plasma-incomingmsg is a plasmoid that notifies you about new messages
 * it is designed to be used on a locked screen so you don't have to unlock
 * to check them.
 *
 */

#include "incomingmsg.h"

#include <QtCore/QProcess>
#include <QtDBus/QDBusInterface>
#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusConnectionInterface>
#include <QtDBus/QDBusReply>
#include <QtGui/QFontMetrics>
#include <QtGui/QFrame>
#include <QtGui/QLabel>
#include <QtGui/QPainter>

#include <QGraphicsLinearLayout>

#include <Plasma/Label>
#include <Plasma/Svg>

#include <KConfigDialog>
#include <KDebug>
#include <KIcon>
#include <KIconEffect>
#include <KLocale>

IncomingMsg::IncomingMsg(QObject *parent, const QVariantList &args)
        : Plasma::Applet(parent, args),
          mEvolutionLabel(0), mEvolutionIconLabel(0),
          mKMailLabel(0), mKMailIconLabel(0),
          mXChatLabel(0), mXChatIconLabel(0),
          mKopeteLabel(0), mKopeteIconLabel(0),
          mPidginLabel(0), mPidginIconLabel(0),
          mErrorLabel(0), mLayout(0),
          mEvolutionLayout(0), mKMailLayout(0),
          mXChatLayout(0), mKopeteLayout(0),
          mPidginLayout(0)
{
    // this will get us the standard applet background, for free!
    setBackgroundHints(DefaultBackground);
    resize(300, 80);
}

IncomingMsg::~IncomingMsg()
{
    delete mKMailLayout;
    delete mKMailIconLabel;
    delete mKMailLabel;

    delete mXChatLayout;
    delete mXChatIconLabel;
    delete mXChatLabel;

    delete mKopeteLayout;
    delete mKopeteIconLabel;
    delete mKopeteLabel;

    delete mPidginLayout;
    delete mPidginIconLabel;
    delete mPidginLabel;
}

void IncomingMsg::init()
{
    /* initialize layout */
    setHasConfigurationInterface(true);
    KConfigGroup cg = config();
    mShowKMail = cg.readEntry("showKMail", true);
    mShowXChat = cg.readEntry("showXChat", true);
    mShowKopete = cg.readEntry("showKopete", true);
    mShowPidgin = cg.readEntry("showPidgin", true);

    initLayout();
}

void IncomingMsg::initEvolutionLayout()
{
    /* test for the evolution dbus interface */
    // TODO find out why evolution does not expose dbus on a kde session here
//    QDBusInterface evolutionDBusTest( "org.freedesktop.Notification",
//                                      "‘/org/freedesktop/Notifications",
//                                      "org.freedesktop.Notifications" );
//    QDBusReply<QString>evolutionReply = evolutionDBusTest.call( "New Email" );
//    if(/*!evolutionReply.isValid()*/false)
//        kDebug() << "Evolution DBus interface test error: " << evolutionReply.error();
//    else{
//        QDBusConnection mDBus = QDBusConnection::sessionBus();
//
//        if( !mDBus.connect ( "org.gnome.evolution", "/Evolution", "org.gnome.evolution.mail.dbus.Signal",
//                         "Newmail",
//                         this, SLOT(slotNewMail()) ) )
//            kDebug() << "Could not connect Evolution to slot.";
//        else{
//            mEvolutionLayout = new QGraphicsLinearLayout(Qt::Horizontal);
//            mEvolutionLabel = new Plasma::Label(this);
//            mEvolutionLabel->setText( i18n("No new mail."));
//            KIcon icon( "evolution" );
//            mEvolutionIconLabel = new Plasma::Label(this);
//            mEvolutionIconLabel->setMinimumWidth(32);
//            mEvolutionIconLabel->setMinimumHeight(32);
//            KIconEffect effect;
//            mEvolutionIconLabel->nativeWidget()->setPixmap(
//                effect.apply( icon.pixmap(32,32), KIconEffect::ToGray, 1, QColor(),QColor(), true )
//                                                );
//
//            mEvolutionLayout->addItem(mEvolutionIconLabel);
//            mEvolutionLayout->addItem(mEvolutionLabel);
//            mEvolutionLayout->setAlignment(mEvolutionLabel, Qt::AlignLeft);
//
//            mLayout->addItem(mEvolutionLayout);
//        }
//    }
}

void IncomingMsg::initKMailLayout()
{
    /* test for the kmail dbus interface */
    if (mShowKMail) {
        QDBusInterface kmailDBusTest("org.kde.kmail", "/KMail", "org.freedesktop.DBus.Introspectable");
        QDBusReply<QString>kmailReply = kmailDBusTest.call("Introspect");
        if (!kmailReply.isValid())
            kDebug() << "KMail DBus interface test error: " << kmailReply.error();
        else {
            QDBusConnection mDBus = QDBusConnection::sessionBus();

            if (!mDBus.connect("org.kde.kmail", "/KMail", "org.kde.kmail.kmail",
                               "unreadCountChanged",
                               this, SLOT(slotNewKMailMail())))
                kDebug() << "Could not connect KMail to slot.";
            else {
                mKMailLayout = new QGraphicsLinearLayout(Qt::Horizontal);
                mKMailLabel = new Plasma::Label(this);
                mKMailLabel->setText(i18n("No new mail."));
                KIcon icon("kmail");
                mKMailIconLabel = new Plasma::Label(this);
                mKMailIconLabel->setMinimumWidth(32);
                mKMailIconLabel->setMinimumHeight(32);
                KIconEffect effect;
                mKMailIconLabel->nativeWidget()->setPixmap(effect.apply(icon.pixmap(32, 32),
                                                                        KIconEffect::ToGray, 1,
                                                                        QColor(), QColor(), true)
                                                           );

                mKMailLayout->addItem(mKMailIconLabel);
                mKMailLayout->addItem(mKMailLabel);
                mKMailLayout->setAlignment(mKMailLabel, Qt::AlignLeft);

                mLayout->addItem(mKMailLayout);
            }
        }
    }
}

void IncomingMsg::initXChatLayout()
{
    /* test for the xchat dbus interface */
    // do not really understand how this interface works
    // got this working code from http://arstechnica.com/reviews/hardware/tux-droid-review.ars/3
    // we need to hook it up first. this first call is not only for interface testing but also for
    // setup.
    if (mShowXChat) {
        QDBusReply<bool> reply = QDBusConnection::sessionBus().interface()->isServiceRegistered("org.xchat.service");
        if (reply.isValid() && reply.value()){
            QDBusConnection mDBus = QDBusConnection::sessionBus();
            if (!mDBus.connect("org.xchat.service", "/org/xchat/Remote",
                               "org.xchat.plugin", "PrintSignal",
                               this, SLOT(slotNewXChatIM())))
                kDebug() << "Could not connect XChat to slot.";
            else {
                mXChatLayout = new QGraphicsLinearLayout(Qt::Horizontal);
                mXChatLabel = new Plasma::Label(this);
                mXChatLabel->setText(i18n("No new XChat messages."));
                KIcon icon("xchat");
                mXChatIconLabel = new Plasma::Label(this);
                mXChatIconLabel->setMinimumWidth(32);
                mXChatIconLabel->setMinimumHeight(32);
                KIconEffect effect;
                mXChatIconLabel->nativeWidget()->setPixmap(
                    effect.apply(icon.pixmap(32, 32), KIconEffect::ToGray, 1, QColor(), QColor(), true)
                );

                mXChatLayout->addItem(mXChatIconLabel);
                mXChatLayout->addItem(mXChatLabel);
                mXChatLayout->setAlignment(mXChatLabel, Qt::AlignLeft);

                mLayout->addItem(mXChatLayout);
            }
        }
    }
}

void IncomingMsg::initKopeteLayout()
{
    /* test for the kopete dbus interface */
    if (mShowKopete) {
        QDBusInterface kopeteDBusTest("org.kde.kopete", "/kopete", "org.freedesktop.DBus.Introspectable");
        QDBusReply<QString>kopeteReply = kopeteDBusTest.call("Introspect");
        if (!kopeteReply.isValid())
            kDebug() << "Kopete DBus interface test error: " << kopeteReply.error();
        else {
            QDBusConnection mDBus = QDBusConnection::sessionBus();

            if (!mDBus.connect("org.kde.kopete", "/Kopete", "org.kde.Kopete",
                               "contactChanged",
                               this, SLOT(slotNewKopeteIM(const QString&))))
                kDebug() << "Could not connect Kopete to slot.";
            else {
                mKopeteLayout = new QGraphicsLinearLayout(Qt::Horizontal);
                mKopeteLabel = new Plasma::Label(this);
                mKopeteLabel->setText(i18n("No new Kopete messages."));
                KIcon icon("kopete");
                mKopeteIconLabel = new Plasma::Label(this);
                mKopeteIconLabel->setMinimumWidth(32);
                mKopeteIconLabel->setMinimumHeight(32);
                KIconEffect effect;
                mKopeteIconLabel->nativeWidget()->setPixmap(
                    effect.apply(icon.pixmap(32, 32), KIconEffect::ToGray, 1, QColor(), QColor(), true)
                );

                mKopeteLayout->addItem(mKopeteIconLabel);
                mKopeteLayout->addItem(mKopeteLabel);
                mKopeteLayout->setAlignment(mKopeteLabel, Qt::AlignLeft);

                mLayout->addItem(mKopeteLayout);
            }
        }
    }
}

void IncomingMsg::initPidginLayout()
{
    /* test for the pidgin dbus interface */
    // FIXME introspect does not work here with qdbus trying sth. else
    if (mShowPidgin) {
        QDBusInterface pidginDBusTest("im.pidgin.purple.PurpleService", "/im/pidgin/purple/PurpleObject",
                                      "im.pidgin.purple.PurpleInterface");
        QDBusReply<QString> pidginReply = pidginDBusTest.call("PurpleBuddyGetName", int(0));
        if (!pidginReply.isValid())
            kDebug() << "Pidgin DBus interface test error: " << pidginReply.error();
        else {
            QDBusConnection mDBus = QDBusConnection::sessionBus();

            if (!mDBus.connect("im.pidgin.purple.PurpleService", "/im/pidgin/purple/PurpleObject",
                               "im.pidgin.purple.PurpleInterface", "ReceivedImMsg",
                               this, SLOT(slotNewPidginIM())))
                kDebug() << "Could not connect to Pidgin on DBus.";
            else {
                mPidginLayout = new QGraphicsLinearLayout(Qt::Horizontal);
                mPidginLabel = new Plasma::Label(this);
                mPidginLabel->setText(i18n("No new Pidgin messages."));
                KIcon icon("pidgin");
                mPidginIconLabel = new Plasma::Label(this);
                mPidginIconLabel->setMinimumWidth(32);
                mPidginIconLabel->setMinimumHeight(32);
                KIconEffect effect;
                mPidginIconLabel->nativeWidget()->setPixmap(
                    effect.apply(icon.pixmap(32, 32), KIconEffect::ToGray, 1, QColor(), QColor(), true)
                );

                mPidginLayout->addItem(mPidginIconLabel);
                mPidginLayout->addItem(mPidginLabel);
                mPidginLayout->setAlignment(mPidginLabel, Qt::AlignLeft);

                mLayout->addItem(mPidginLayout);
            }
        }
    }
}

void IncomingMsg::clearLayout()
{
    delete mKMailLayout;
    mKMailLayout = NULL;
    delete mKMailIconLabel;
    mKMailIconLabel = NULL;
    delete mKMailLabel;
    mKMailLabel = NULL;

    delete mXChatLayout;
    mXChatLayout = NULL;
    delete mXChatIconLabel;
    mXChatIconLabel = NULL;
    delete mXChatLabel;
    mXChatLabel = NULL;

    delete mKopeteLayout;
    mKopeteLayout = NULL;
    delete mKopeteIconLabel;
    mKopeteIconLabel = NULL;
    delete mKopeteLabel;
    mKopeteLabel = NULL;

    delete mPidginLayout;
    mPidginLayout = NULL;
    delete mPidginIconLabel;
    mPidginIconLabel = NULL;
    delete mPidginLabel;
    mPidginLabel = NULL;

    delete mErrorLabel;
    mErrorLabel = NULL;
}

void IncomingMsg::initLayout()
{
    mLayout = new QGraphicsLinearLayout(Qt::Vertical);

    //initEvolutionLayout();
    initKMailLayout();
    initXChatLayout();
    initKopeteLayout();
    initPidginLayout();

    if (!mLayout->count()) {
        mErrorLabel = new Plasma::Label();
        mErrorLabel->setText(i18n("No running messaging apps found. Supported apps are %1, %2, %3, %4.",
                                  QString("KMail"), QString("XChat"), QString("Kopete"),
                                  QString("Pidgin")));
        mLayout->addItem(mErrorLabel);
    }

    setLayout(mLayout);
}

void IncomingMsg::createConfigurationInterface(KConfigDialog *dialog)
{
    QWidget *widget = new QWidget();
    ui.setupUi(widget);

    KConfigGroup cg = config();
    ui.showKMail->setChecked(cg.readEntry("showKMail", true));
    ui.showXChat->setChecked(cg.readEntry("showXChat", true));
    ui.showKopete->setChecked(cg.readEntry("showKopete", true));
    ui.showPidgin->setChecked(cg.readEntry("showPidgin", true));

    connect(dialog, SIGNAL(applyClicked()), this, SLOT(configAccepted()));
    connect(dialog, SIGNAL(okClicked()), this, SLOT(configAccepted()));

    dialog->addPage(widget, i18n("General"), icon());
}


void IncomingMsg::configAccepted()
{
    mShowKMail = ui.showKMail->isChecked();
    mShowXChat = ui.showXChat->isChecked();
    mShowKopete = ui.showKopete->isChecked();
    mShowPidgin = ui.showPidgin->isChecked();

    KConfigGroup cg = config();
    cg.writeEntry("showKMail", ui.showKMail->isChecked());
    cg.writeEntry("showXChat", ui.showXChat->isChecked());
    cg.writeEntry("showKopete", ui.showKopete->isChecked());
    cg.writeEntry("showPidgin", ui.showPidgin->isChecked());

    clearLayout();
    initLayout();
}

void IncomingMsg::constraintsEvent(Plasma::Constraints constraints)
{
    if (mLayout && (constraints & Plasma::SizeConstraint ||
                    constraints & Plasma::FormFactorConstraint)) {
        QRectF layoutRectF = mLayout->geometry();
        QRectF appletRectF = geometry();
        appletRectF.setHeight(layoutRectF.height() + 10);

        if (formFactor() == Plasma::Vertical) {
            setMinimumSize(0, appletRectF.height());
        } else if (formFactor() == Plasma::Horizontal) {
            setMinimumSize(appletRectF.width(), 0);
        } else if (layoutRectF.height() > appletRectF.height()) {
            setMinimumSize(appletRectF.size());
        }
    }
}

void IncomingMsg::slotNewEvolutionMail()
{
    KIcon icon("evolution");
    mEvolutionIconLabel->nativeWidget()->setPixmap(icon.pixmap(32, 32));
    mEvolutionLabel->setText(i18n("Your Evolution mail count has changed."));
}

void IncomingMsg::slotNewKMailMail()
{
    KIcon icon("kmail");
    mKMailIconLabel->nativeWidget()->setPixmap(icon.pixmap(32, 32));
    mKMailLabel->setText(i18n("Your KMail mail count has changed."));
}

void IncomingMsg::slotNewXChatIM()
{
    KIcon icon("xchat");
    mXChatIconLabel->nativeWidget()->setPixmap(icon.pixmap(32, 32));
    mXChatLabel->setText(i18n("You have new XChat messages."));
}

void IncomingMsg::slotNewKopeteIM(const QString& contactId)
{
    QDBusInterface kopeteDBusTest("org.kde.kopete", "/Kopete", "org.kde.Kopete");
    QDBusReply<QVariantMap>kopeteReply = kopeteDBusTest.call("contactProperties", contactId);
    if (kopeteReply.isValid()
            && !kopeteReply.value()["pending_messages"].toStringList().empty()) {
        KIcon icon("kopete");
        mKopeteIconLabel->nativeWidget()->setPixmap(icon.pixmap(32, 32));
        mKopeteLabel->setText(i18n("You have new Kopete messages."));
    }
}

void IncomingMsg::slotNewPidginIM()
{
    KIcon icon("pidgin");
    mPidginIconLabel->nativeWidget()->setPixmap(icon.pixmap(32, 32));
    mPidginLabel->setText(i18n("You have new Pidgin messages."));
}

#include "incomingmsg.moc"
