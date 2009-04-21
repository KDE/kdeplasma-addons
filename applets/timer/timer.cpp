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

#include <QGraphicsSceneMouseEvent>
#include <QTimer>

#include <KConfigDialog>
#include <KDebug>
#include <KNotification>
#include <KShell>
#include <KToolInvocation>

#include <Plasma/Label>
#include <Plasma/Svg>
#include <Plasma/SvgWidget>

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
    saveTimer();
}

void Timer::init()
{
    m_svg = new Plasma::Svg(this);
    m_svg->setImagePath("widgets/timer");
    m_svg->setContainsMultipleImages(true);

    // Choose graphical separator based on the text one.
    m_separatorBasename = QString("separator");
    QString textSeparator = CustomTimeEditor::timerSeparator().remove(' ');
    if (textSeparator == QString('.')) {
        m_separatorBasename += 'B';
    } else if (textSeparator == QString(' ')) {
        m_separatorBasename += 'C';
    }

    m_hoursDigit[0] = new TimerDigit(m_svg, 36000, this);
    m_hoursDigit[1] = new TimerDigit(m_svg, 3600, this);
    m_minutesDigit[0] = new TimerDigit(m_svg, 600, this);
    m_minutesDigit[1] = new TimerDigit(m_svg, 60, this);
    m_secondsDigit[0] = new TimerDigit(m_svg, 10, this);
    m_secondsDigit[1] = new TimerDigit(m_svg, 1, this);
    m_separator[0] = new Plasma::SvgWidget(m_svg, m_separatorBasename, this);
    m_separator[1] = new Plasma::SvgWidget(m_svg, m_separatorBasename, this);
    m_title = new Plasma::Label(this);
    m_title->setAlignment(Qt::AlignTop | Qt::AlignHCenter);

    connect(m_hoursDigit[0], SIGNAL(changed(int)), this, SLOT(digitChanged(int)));
    connect(m_hoursDigit[1], SIGNAL(changed(int)), this, SLOT(digitChanged(int)));
    connect(m_minutesDigit[0], SIGNAL(changed(int)), this, SLOT(digitChanged(int)));
    connect(m_minutesDigit[1], SIGNAL(changed(int)), this, SLOT(digitChanged(int)));
    connect(m_secondsDigit[0], SIGNAL(changed(int)), this, SLOT(digitChanged(int)));
    connect(m_secondsDigit[1], SIGNAL(changed(int)), this, SLOT(digitChanged(int)));

    KConfigGroup cg = config();
    m_predefinedTimers = cg.readEntry("predefinedTimers", QStringList() << "00:00:30" << "00:01:00"
                                                       << "00:02:00" << "00:05:00" << "00:07:30"
                                                       << "00:10:00" << "00:15:00" << "00:20:00"
                                                       << "00:25:00" << "00:30:00" << "00:45:00"
                                                       << "01:00:00");
    m_title->setVisible(cg.readEntry("showTitle", false));
    m_title->setText(cg.readEntry("title", i18n("Timer")));

    bool hideSeconds = cg.readEntry("hideSeconds", false);
    m_secondsDigit[0]->setVisible(!hideSeconds);
    m_secondsDigit[1]->setVisible(!hideSeconds);
    m_separator[1]->setVisible(!hideSeconds);

    m_showMessage = cg.readEntry("showMessage", true);
    m_message = cg.readEntry("message", i18n("Timer Timeout"));
    m_runCommand = cg.readEntry("runCommand", false);
    m_command = cg.readEntry("command", "");

    // Timers are kept non-localized in the config, to work across language changes.
    QStringList localizedTimers;
    foreach (const QString &timer, m_predefinedTimers) {
        localizedTimers.append(CustomTimeEditor::toLocalizedTimer(timer));
    }
    m_predefinedTimers = localizedTimers;

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

    m_running = cg.readEntry("running", false);
    if (m_running){
        QDateTime startedAt = cg.readEntry("startedAt", QDateTime::currentDateTime());
        int tmpSeconds = cg.readEntry("seconds", 0) - startedAt.secsTo(QDateTime::currentDateTime());
        if (tmpSeconds > 0){
            setSeconds(tmpSeconds);
            startTimer();
        }else{
            //TODO: We should notify user about expired timer
            m_running = false;
        }
    }else{
        setSeconds(cg.readEntry("seconds", 0));
        if (m_seconds){
            m_startAction->setEnabled(true);
            m_resetAction->setEnabled(true);
        }
    }
}

void Timer::constraintsEvent(Plasma::Constraints constraints)
{
    Q_UNUSED(constraints)

    int appletHeight = (int) contentsRect().height();
    int appletWidth = (int) contentsRect().width();
    float digits = m_secondsDigit[0]->isVisible() ? 7 : 4.5;

    int h = (int) ((appletHeight / 2) * digits < appletWidth ? appletHeight : ((appletWidth - (digits - 1)) / digits) * 2);
    int w = h / 2;
    int y = (int) (contentsRect().y() + (appletHeight - h) / 2);
    int x = (int) (contentsRect().x() + (appletWidth - w * digits) / 2);

    m_hoursDigit[0]->setGeometry(x, y, w, h);
    m_hoursDigit[1]->setGeometry(x + w, y, w, h);  

    m_separator[0]->setGeometry(x + (w * 2), y, w/2, h);

    m_minutesDigit[0]->setGeometry(x + (w * 2) + (w/2), y, w, h);
    m_minutesDigit[1]->setGeometry(x + (w * 3) + (w/2), y, w, h);

    m_separator[1]->setGeometry(x + (w * 4) + (w/2), y, w/2, h);

    m_secondsDigit[0]->setGeometry(x + (w * 5), y, w, h);
    m_secondsDigit[1]->setGeometry(x + (w * 6), y, w, h);

    QFont font = this->font();
    font.setPixelSize( y - 6 );
    m_title->nativeWidget()->setFont( font );
    m_title->setGeometry(QRectF(0, 4, appletWidth, y - 2));
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
        action->setProperty("seconds", QTime(0, 0, 0).secsTo(QTime::fromString(*it, CustomTimeEditor::toLocalizedTimer(CustomTimeEditor::TIME_FORMAT))));
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

    parent->addPage(widget, i18n("General"), icon());
    parent->addPage(predefinedTimersWidget, i18n("Predefined Timers"), icon());
    connect(parent, SIGNAL(applyClicked()), this, SLOT(configAccepted()));
    connect(parent, SIGNAL(okClicked()), this, SLOT(configAccepted()));

    ui.showTitleCheckBox->setChecked(m_title->isVisible());
    ui.titleLineEdit->setEnabled(m_title->isVisible());
    ui.titleLineEdit->setText(m_title->text());
    ui.hideSecondsCheckBox->setChecked(!m_secondsDigit[0]->isVisible());
    ui.showMessageCheckBox->setChecked(m_showMessage);
    ui.messageLineEdit->setEnabled(m_showMessage);
    ui.messageLineEdit->setText(m_message);
    ui.runCommandCheckBox->setChecked(m_runCommand);
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
    QStringList unlocalizedTimers;
    foreach (const QString &timer, m_predefinedTimers) {
        unlocalizedTimers.append(CustomTimeEditor::fromLocalizedTimer(timer));
    }
    cg.writePathEntry("predefinedTimers", unlocalizedTimers);

    m_title->setVisible(ui.showTitleCheckBox->isChecked());
    cg.writeEntry("showTitle", m_title->isVisible());

    bool hideSeconds = ui.hideSecondsCheckBox->isChecked();
    m_secondsDigit[0]->setVisible(!hideSeconds);
    m_secondsDigit[1]->setVisible(!hideSeconds);
    m_separator[1]->setVisible(!hideSeconds);
    cg.writeEntry("hideSeconds", hideSeconds);

    m_title->setText(ui.titleLineEdit->text());
    cg.writeEntry("title", m_title->text());

    m_showMessage = ui.showMessageCheckBox->isChecked();
    cg.writeEntry("showMessage", m_showMessage);

    m_message = ui.messageLineEdit->text();
    cg.writeEntry("message", m_message);

    m_runCommand = ui.runCommandCheckBox->isChecked();
    cg.writeEntry("runCommand", m_runCommand);

    m_command = ui.commandLineEdit->text();
    cg.writeEntry("command", m_command);

    createMenuAction();
    updateConstraints(Plasma::SizeConstraint);
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

    if (m_seconds != 0){
        setSeconds(--m_seconds);
    }
}

void Timer::setSeconds(int secs)
{
    m_seconds = secs;

    int hours =  m_seconds / (60 * 60);
    int mins = (m_seconds % (60 * 60)) / 60;
    int seconds =  m_seconds % 60;

    QString suffix = (m_seconds < 60 && m_running) ? "_1" : "";

    if ((m_seconds >= 60) || m_secondsDigit[0]->isVisible()){
        m_hoursDigit[0]->setElementID(QString::number(hours / 10) + suffix);
        m_hoursDigit[1]->setElementID(QString::number(hours % 10) + suffix);

        m_separator[0]->setElementID(m_separatorBasename + suffix);

        m_minutesDigit[0]->setElementID(QString::number(mins / 10) + suffix);
        m_minutesDigit[1]->setElementID(QString::number(mins % 10) + suffix);

        m_separator[1]->setElementID(m_separatorBasename + suffix);

        m_secondsDigit[0]->setElementID(QString::number(seconds / 10) + suffix);
        m_secondsDigit[1]->setElementID(QString::number(seconds % 10) + suffix);
    }else{
        m_hoursDigit[0]->setElementID(QString::number(hours / 10) + suffix);
        m_hoursDigit[1]->setElementID(QString::number(hours % 10) + suffix);

        m_separator[0]->setElementID(m_separatorBasename + suffix);

        m_minutesDigit[0]->setElementID(QString::number(seconds / 10) + suffix);
        m_minutesDigit[1]->setElementID(QString::number(seconds % 10) + suffix);
    }
}

void Timer::slotCountDone()
{
    if (m_showMessage){
        //TODO: probably something with an OK button is better.
        if (m_title->isVisible())
          KNotification::event(KNotification::Notification, m_title->text() + " - " + m_message);
        else
          KNotification::event(KNotification::Notification, m_message);
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

void Timer::saveTimer()
{
    KConfigGroup cg = config();
    cg.writeEntry("running", m_running);
    cg.writeEntry("startedAt", QDateTime::currentDateTime());
    cg.writeEntry("seconds", m_seconds);

    emit configNeedsSaving();
}

void Timer::startTimer()
{
    m_running = true;

    saveTimer();

    timer.start(1000);

    m_startAction->setEnabled(false);
    m_stopAction->setEnabled(true);
}


void Timer::stopTimer()
{
    m_running = false;

    saveTimer();

    timer.stop();

    m_startAction->setEnabled(true);
    m_stopAction->setEnabled(false);
}

void Timer::resetTimer()
{
    stopTimer();

    saveTimer();

    setSeconds(0);
    m_resetAction->setEnabled(false);
    m_startAction->setEnabled(false);
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
    setSeconds(action->property("seconds").toInt());

    startTimer();
}

void Timer::digitChanged(int value)
{
    if (m_running) return;

    if (value < 0){
        if (m_seconds >= abs(value)){
            setSeconds((m_seconds - abs(value)) % 86400);
        }
    }else{
        setSeconds((m_seconds + abs(value)) % 86400);
    }

    if (m_seconds != 0){
        m_startAction->setEnabled(true);
        m_resetAction->setEnabled(true);
    }else{
        m_startAction->setEnabled(false);
        m_resetAction->setEnabled(false);
    }
}

#include "timer.moc"
