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
 * plasma-leavemsg is a plasmoid that send persistent messages to knotify
 *
 */

#include <QtCore/QProcess>
#include <QtCore/QHash>
#include <QtDBus/QDBusInterface>
#include <QtDBus/QDBusReply>
#include <QtGui/QFontMetrics>
#include <QtGui/QFrame>
#include <QtGui/QPainter>
#include <QGraphicsLinearLayout>

#include <Plasma/Label>
#include <Plasma/PushButton>
#include <Plasma/Svg>
#include <Plasma/TextEdit>

#include <KConfigDialog>
#include <KDateTime>
#include <KDebug>
#include <KPushButton>
#include <KTextEdit>
#include <KLocale>

#include "leavenote.h"

#define MAX_CHAR 1000

LeaveNote::LeaveNote(QObject *parent, const QVariantList &args)
        : Plasma::Applet(parent, args),
        mTheme(this), mTextEdit(0),
        mTimer(0),
        mUseKNotes(false), mUseNotification(false)
{
    // this will get us the standard applet background, for free!
    setBackgroundHints(NoBackground);
    resize(340, 250);
    setMinimumSize(300, 200); // replace with correct numbers
}

LeaveNote::~LeaveNote()
{
}

void LeaveNote::init()
{
    /* do config stuff */
    setHasConfigurationInterface(true);
    KConfigGroup cg = config();
    mUseKNotes = cg.readEntry("useKNotes", true);

    /* initialize layout */
    mTheme.setImagePath("widgets/notes");
    mTheme.setContainsMultipleImages(false);

    mLabel = new Plasma::Label(this);
    mLabel->setText(i18n("Leave me a note: "));
    mLabel->setStyleSheet("color: black");
    mTextEdit = new Plasma::TextEdit(this);
    mTextEdit->setMinimumSize(QSize(0, 0));
    mTextEdit->nativeWidget()->setAcceptRichText(false);
    mTextEdit->nativeWidget()->viewport()->setAutoFillBackground(false);
    connect(mTextEdit, SIGNAL(textChanged()), SLOT(slotLimitMessageLength()));

    mSendButton = new Plasma::PushButton(this);
    mSendButton->setText(i18n("Send"));
    mSendButton->setStyleSheet("background: rgba( 220, 220, 220, 30% )");
    mSendButton->nativeWidget()->setAutoFillBackground(false);
    connect(mSendButton, SIGNAL(clicked()), SLOT(slotSend()));

    mLayout = new QGraphicsLinearLayout(Qt::Vertical);

    mLayout->addItem(mLabel);
    mLayout->addItem(mTextEdit);
    mLayout->addItem(mSendButton);
    // FIXME Why does this not work?
    mLayout->setAlignment(mSendButton, Qt::AlignRight);

    setLayout(mLayout);
}

void LeaveNote::paintInterface(QPainter *p,
                               const QStyleOptionGraphicsItem *option,
                               const QRect &contentsRect)
{
    Q_UNUSED(option);
    mTheme.paint(p, contentsRect);
}

void LeaveNote::constraintsEvent(Plasma::Constraints constraints)
{
    setBackgroundHints(Plasma::Applet::NoBackground);
    if (constraints & Plasma::SizeConstraint) {
        mTheme.resize(geometry().size());
        mLayout->setContentsMargins(.08*geometry().width(),
                                    .08*geometry().height(),
                                    .08*geometry().width(),
                                    .08*geometry().height());
    }
}

void LeaveNote::createConfigurationInterface(KConfigDialog *dialog)
{
    QWidget *widget = new QWidget();
    ui.setupUi(widget);
    dialog->setMainWidget(widget);

    KConfigGroup cg = config();
    ui.useKNotesCheckBox->setChecked(cg.readEntry("useKNotes", true));

    dialog->setButtons(KDialog::Ok | KDialog::Cancel);  // | KDialog::Apply
    connect(dialog, SIGNAL(applyClicked()), this, SLOT(configAccepted()));
    connect(dialog, SIGNAL(okClicked()), this, SLOT(configAccepted()));

    dialog->addPage(widget, i18n("General"), icon());
}


void LeaveNote::configAccepted()
{
    mUseKNotes = ui.useKNotesCheckBox->isChecked();

    KConfigGroup cg = config();
    cg.writeEntry("useKNotes", ui.useKNotesCheckBox->isChecked());
}

bool LeaveNote::checkKNotesDBusInterface()
{
    QDBusInterface knotesDBusTest("org.kde.knotes", "/KNotes", "org.freedesktop.DBus.Introspectable");
    QDBusReply<QString>reply = knotesDBusTest.call("Introspect");

    bool valid = reply.isValid();
    if (!valid)
        kDebug() << "KNotes DBus interface test error: " << reply.error();

    return valid;
}

void LeaveNote::slotSend()
{
    if (mTextEdit->nativeWidget()->toPlainText().isEmpty())
        return;

    QString time = KGlobal::locale()->formatTime(KDateTime::currentLocalDateTime().time());
    QString title = i18nc("String + time", "Somebody has left a note at %1", time);
    QString msg = mTextEdit->text();

    /* load KNotes if desired and unloaded */
    if (mUseKNotes && !checkKNotesDBusInterface()) {
        QHash<QString, QVariant> hash;
        hash["title"] = title;
        hash["msg"] = msg;
        mMsgCache << hash;

        /* check if we are loading KNotes already then cache the msg*/
        if (mTimer && mTimer->isActive()) {
            return;
        }

        // TODO fine grain for errors after start of KNotes
        QProcess knotes(this);
        knotes.startDetached("knotes", QStringList() << "--skip-note");
        knotes.waitForStarted();

        /* wait for the dbus interface to settle */
        mTimer->singleShot(10000, this, SLOT(slotWaitForKNotes()));

    } else {
        createNote(title, msg);
    }

    mTextEdit->setText(QString());
}

void LeaveNote::slotLimitMessageLength()
{
    if (mTextEdit->nativeWidget()->toPlainText().count() < MAX_CHAR) return;

    KTextEdit* textEdit = mTextEdit->nativeWidget();

    QString text = textEdit->toPlainText();
    text.resize(MAX_CHAR - 1);
    QTextCursor cursor = textEdit->textCursor();
    cursor.setPosition(cursor.position() - 1);
    textEdit->setText(text);
    textEdit->setTextCursor(cursor);
}

void LeaveNote::slotWaitForKNotes()
{
    kDebug();
    if(!checkKNotesDBusInterface()){
        // disable KNotes as it has not loaded yet
        mUseKNotes = false;
    }

    /* now we can empty the message cache */
    QList<QHash<QString, QVariant> >::iterator i;
    for (i = mMsgCache.begin(); i != mMsgCache.end(); ++i) {
        createNote((*i)["title"].toString(),
                   (*i)["msg"].toString());
    }
    mMsgCache.clear();
}

void LeaveNote::createNote(const QString& title, const QString& msg)
{
    kDebug();
    if (mUseKNotes) {
        QDBusInterface knotesDBus("org.kde.knotes", "/KNotes", "org.kde.KNotes");
        knotesDBus.call("newNote", title, msg);
    } else {
        QWidget* widget = new QWidget(0);
        widget->setWindowTitle(title);
        KTextEdit *msgTextEdit = new KTextEdit(msg, widget);
        msgTextEdit->setReadOnly(true);
        QVBoxLayout *layout = new QVBoxLayout();
        layout->addWidget(msgTextEdit);
        widget->setLayout(layout);
        widget->show();
    }
}

#include "leavenote.moc"
