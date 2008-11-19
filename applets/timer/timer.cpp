/***************************************************************************
 *   Copyright 2008 by Davide Bettio <davide.bettio@kdemail.net>           *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

#include "timer.h"

#include <QDBusMessage>
#include <QDBusConnection>
#include <QGraphicsView>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QTimer>
#include <KDebug>
#include <QMenu>

#include <KApplication>
#include <KConfigDialog>
#include <KPassivePopup>
#include <KShell>
#include <KToolInvocation>

#include <Plasma/Svg>
#include <Plasma/Theme>
#include "customtimeeditor.h"



Timer::Timer(QObject *parent, const QVariantList &args)
    : Plasma::Applet(parent, args),
      m_seconds(),
      m_running(false)
{
    resize(315, 125);
    setHasConfigurationInterface(true);
    setAspectRatioMode(Plasma::KeepAspectRatio);
}

Timer::~Timer()
{
}

void Timer::init()
{
    m_svg = new Plasma::Svg(this);
    m_svg->setImagePath("widgets/timer");
    m_svg->setContainsMultipleImages(true);

    KConfigGroup cg = config();
    m_predefinedTimers = cg.readEntry("predefinedTimers", QStringList() << "00:00:30" << "00:01:00"
                                                       << "00:02:00" << "00:05:00" << "00:07:30"
                                                       << "00:10:00" << "00:15:00" << "00:20:00"
                                                       << "00:25:00" << "00:30:00" << "00:45:00"
                                                       << "01:00:00");
    m_showMessage = cg.readEntry("showMessage", true);
    m_message = cg.readEntry("message", i18n("Timer Timeout"));
    m_runCommand = cg.readEntry("runCommand", false);
    m_command = cg.readEntry("command", "");

    connect(&timer, SIGNAL(timeout()), this, SLOT(updateTimer()));

    m_startAction = new QAction(i18n("Start"), this);
    m_startAction->setEnabled(false);
    connect(m_startAction, SIGNAL(triggered(bool)), this, SLOT(startTimer()));

    m_stopAction = new QAction(i18n("Stop"), this);
    m_stopAction->setEnabled(false);
    connect(m_stopAction, SIGNAL(triggered(bool)), this, SLOT(stopTimer()));

    m_resetAction = new QAction(i18n("Reset"), this);
    m_resetAction->setEnabled(false);
    connect(m_resetAction, SIGNAL(triggered(bool)), this, SLOT(resetTimer()));
    createMenuAction();
}

void Timer::createMenuAction()
{
    //necessary when we change predefined timer in config dialogbox
    actions.clear();
    qDeleteAll( actions );
    actions.append(m_startAction);
    actions.append(m_stopAction);
    actions.append(m_resetAction);

    QAction *separator0 = new QAction(this);
    separator0->setSeparator(true);
    actions.append(separator0);

    QAction *action = 0;

    QStringList::const_iterator end =  m_predefinedTimers.constEnd();
    lstActionTimer = new QActionGroup(this);
    for (QStringList::const_iterator it = m_predefinedTimers.constBegin(); it != end; ++it) {
        action = new QAction(*it, this);
        action->setProperty("seconds", QTime(0, 0, 0).secsTo(QTime::fromString(*it, CustomTimeEditor::TIME_FORMAT)));
        lstActionTimer->addAction(action);
        connect(action, SIGNAL(triggered(bool)), this, SLOT(startTimerFromAction()));
        actions.append(action);
    }
    QAction *separator1 = new QAction(this);
    separator1->setSeparator(true);
    actions.append(separator1);
}

void Timer::createConfigurationInterface(KConfigDialog *parent)
{
    QWidget *widget = new QWidget();
    ui.setupUi(widget);

    QWidget *predefinedTimersWidget = new QWidget();
    predefinedTimersUi.setupUi(predefinedTimersWidget);

    parent->setButtons( KDialog::Ok | KDialog::Cancel | KDialog::Apply );
    parent->addPage(widget, i18n("General"), icon());
    parent->addPage(predefinedTimersWidget, i18n("Predefined Timers"), icon());
    connect(parent, SIGNAL(applyClicked()), this, SLOT(configAccepted()));
    connect(parent, SIGNAL(okClicked()), this, SLOT(configAccepted()));

    ui.showMessageCheckBox->setChecked(m_showMessage);
    ui.messageLabel->setEnabled(m_showMessage);
    ui.messageLineEdit->setEnabled(m_showMessage);
    ui.messageLineEdit->setText(m_message);
    ui.runCommandCheckBox->setChecked(m_runCommand);
    ui.commandLabel->setEnabled(m_runCommand);
    ui.commandLineEdit->setEnabled(m_runCommand);
    ui.commandLineEdit->setText(m_command);

    CustomTimeEditor *aCustomEditor = new CustomTimeEditor();
    predefinedTimersUi.defaulttimers->setCustomEditor(*(aCustomEditor->getCustomEditor()));
    predefinedTimersUi.defaulttimers->setItems(m_predefinedTimers);
}

void Timer::configAccepted()
{
    KConfigGroup cg = config();

    m_predefinedTimers = predefinedTimersUi.defaulttimers->items();
    cg.writePathEntry("predefinedTimers", m_predefinedTimers);

    m_showMessage = ui.showMessageCheckBox->isChecked();
    cg.writeEntry("showMessage", m_showMessage);

    m_message = ui.messageLineEdit->text();
    cg.writeEntry("message", m_message);

    m_runCommand = ui.runCommandCheckBox->isChecked();
    cg.writeEntry("runCommand", m_runCommand);

    m_command = ui.commandLineEdit->text();
    cg.writeEntry("command", m_command);

    createMenuAction();
    emit configNeedsSaving();
}

void Timer::updateTimer()
{
    if (m_seconds < 2) {
        timer.stop();
        m_running = false;

        m_startAction->setEnabled(false);
        m_stopAction->setEnabled(false);
        m_resetAction->setEnabled(false);

        slotCountDone();
    }

    if (m_seconds != 0) m_seconds--;

    update();
}

void Timer::slotCountDone()
{
    if (m_showMessage){
        //TODO: probably something with an OK button is better.
        KPassivePopup::message(m_message, (QWidget *) 0);
    }

    if (m_runCommand && !m_command.isEmpty()){
        QStringList args = KShell::splitArgs(m_command);
        QString command = args[0];
        args.removeFirst();
        KToolInvocation::kdeinitExec(command, args);
    }
}

void Timer::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *)
{
    if (!m_running){
        startTimer();
    }else{
        stopTimer();
    }
}

void Timer::startTimer()
{
    m_running = true;

    timer.start(1000);

    m_startAction->setEnabled(false);
    m_stopAction->setEnabled(true);
}


void Timer::stopTimer()
{
    m_running = false;

    timer.stop();

    m_startAction->setEnabled(true);
    m_stopAction->setEnabled(false);
}

void Timer::resetTimer()
{
    stopTimer();

    m_seconds = 0;
    m_resetAction->setEnabled(false);
    m_startAction->setEnabled(false);

    update();
}

void Timer::mousePressEvent(QGraphicsSceneMouseEvent *)
{

}

QList<QAction*> Timer::contextualActions()
{
    lstActionTimer->setEnabled( !m_running );
    return actions;
}

void Timer::startTimerFromAction()
{
    QAction *action = dynamic_cast<QAction*> (sender());
    if (!action || action->property("seconds").type() != QVariant::Int ) return;
    m_seconds=action->property("seconds").toInt();

    startTimer();
}

void Timer::wheelEvent(QGraphicsSceneWheelEvent * event)
{
    Q_UNUSED(event);

    if (m_running) return;

    int delta = 0;

    int appletHeight = (int) contentsRect().height();
    int appletWidth = (int) contentsRect().width();

    int h = (appletHeight / 2) * 7 < appletWidth ? appletHeight : ((appletWidth - 6) / 7) * 2;
    int w = h / 2;
    int y = ((int) contentsRect().y()) + (appletHeight - h) / 2;
    int x = ((int) contentsRect().x()) + (appletWidth - w * 7) / 2;

    if (!(event->pos().y() > y && event->pos().y() < y + h)) return;

    if ((event->pos().x() > x) && (event->pos().x() < x + w)){
        delta = 36000;

    }else if ((event->pos().x() > x + w) && (event->pos().x() < x + (w * 2))){
        delta = 3600;

    }else if ((event->pos().x() > x + (w * 2) + (w / 2)) && (event->pos().x() < x + (w * 3) + (w / 2))){
        delta = 600;

    }else if ((event->pos().x() > x + (w * 3) + (w / 2)) && (event->pos().x() < x + (w * 4) + (w / 2))){
        delta = 60;

    }else if ((event->pos().x() > x + (w * 5)) && (event->pos().x() < x + (w * 6))){
        delta = 10;

    }else if ((event->pos().x() > x + (w * 6)) && (event->pos().x() < x + (w * 7))){
        delta = 1;
    }

    if (event->delta() < 0){
        if (m_seconds >= delta){
            m_seconds = (m_seconds - delta) % 86400;
        }
    }else{
        m_seconds = (m_seconds + delta) % 86400;
    }

    update();

    if (m_seconds != 0){
        m_startAction->setEnabled(true);
        m_resetAction->setEnabled(true);
    }else{
        m_startAction->setEnabled(false);
        m_resetAction->setEnabled(false);
    }
}


void Timer::paintInterface(QPainter *p,
        const QStyleOptionGraphicsItem *option, const QRect &contentsRect)
{
    Q_UNUSED(option);

    int appletHeight = (int) contentsRect.height();
    int appletWidth = (int) contentsRect.width();

    int h = (appletHeight / 2) * 7 < appletWidth ? appletHeight : ((appletWidth - 6) / 7) * 2;
    int w = h / 2;
    int y = contentsRect.y() + (appletHeight - h) / 2;
    int x = contentsRect.x() + (appletWidth - w * 7) / 2;

    int hours =  m_seconds / (60*60);
    int mins = (m_seconds % (60*60)) / 60;
    int seconds =  m_seconds % 60;

    QString suffix = (m_seconds < 60 && m_running) ? "_1" : "";

    m_svg->paint(p, QRectF(x, y, w, h), QString::number(hours / 10) + suffix);
    m_svg->paint(p, QRectF(x + w, y, w, h), QString::number(hours % 10) + suffix);

    m_svg->paint(p, QRectF(x + (w * 2), y, w/2, h), "separator" + suffix);

    m_svg->paint(p, QRectF(x + (w * 2) + (w/2), y, w, h), QString::number(mins / 10) + suffix);
    m_svg->paint(p, QRectF(x + (w * 3) + (w/2), y, w, h), QString::number(mins % 10) + suffix);

    m_svg->paint(p, QRectF(x + (w * 4) + (w/2), y, w/2, h), "separator" + suffix);

    m_svg->paint(p, QRectF(x + (w * 5), y, w, h), QString::number(seconds / 10) + suffix);
    m_svg->paint(p, QRectF(x + (w * 6), y, w, h), QString::number(seconds % 10) + suffix);
}

#include "timer.moc"
