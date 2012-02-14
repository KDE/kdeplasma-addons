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

#include <QSequentialAnimationGroup>
#include <QPropertyAnimation>
#include <QtCore/QEasingCurve>
#include <QtGui/QGraphicsSceneMouseEvent>

#include <KAuthorized>
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
      m_seconds(0),
      m_startingSeconds(0),
      m_running(false),
      m_blinkAnim(0)
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

    connect(&m_timer, SIGNAL(timeout()), this, SLOT(updateTimer()));

    m_startAction = new QAction(i18n("Start"), this);
    m_startAction->setEnabled(false);
    connect(m_startAction, SIGNAL(triggered(bool)), this, SLOT(startTimer()));

    m_stopAction = new QAction(i18n("Stop"), this);
    m_stopAction->setEnabled(false);
    connect(m_stopAction, SIGNAL(triggered(bool)), this, SLOT(stopTimer()));

    m_resetAction = new QAction(i18n("Reset"), this);
    m_resetAction->setEnabled(false);
    connect(m_resetAction, SIGNAL(triggered(bool)), this, SLOT(resetTimer()));

    configChanged();
    createMenuAction();
}

void Timer::configChanged()
{
    KConfigGroup cg = config();
    m_predefinedTimers = cg.readEntry("predefinedTimers", QStringList() << "00:00:30" << "00:01:00"
                                                       << "00:02:00" << "00:05:00" << "00:07:30"
                                                       << "00:10:00" << "00:15:00" << "00:20:00"
                                                       << "00:25:00" << "00:30:00" << "00:45:00"
                                                       << "01:00:00");
    m_showTitle = cg.readEntry("showTitle", false);
    m_timerTitle = cg.readEntry("title", i18n("Timer"));
    m_hideSeconds = cg.readEntry("hideSeconds", false);
    m_showMessage = cg.readEntry("showMessage", true);
    m_message = cg.readEntry("message", i18n("Timer Timeout"));
    m_runCommand = cg.readEntry("runCommand", false);
    m_command = cg.readEntry("command", "");

    m_title->setVisible(m_showTitle);
    m_title->setText(m_timerTitle);

    // update the display for the seconds hiding setting
    m_secondsDigit[0]->setVisible(!m_hideSeconds);
    m_secondsDigit[1]->setVisible(!m_hideSeconds);
    m_separator[1]->setVisible(!m_hideSeconds);
    constraintsEvent(Plasma::SizeConstraint);

    // Timers are kept non-localized in the config, to work across language changes.
    QStringList localizedTimers;
    foreach (const QString &timer, m_predefinedTimers) {
        localizedTimers.append(CustomTimeEditor::toLocalizedTimer(timer));
    }
    m_predefinedTimers = localizedTimers;

    if (isUserConfiguring()) {
        // immediate update of the display
        setSeconds(m_seconds);
        return;
    }

    // everything from here down is run when not called due to the config UI
    m_startedAt = cg.readEntry("startedAt", QDateTime::currentDateTime());
    m_startingSeconds = cg.readEntry("seconds", 0);
    const int runningTime = config().readEntry("running", 0);
    m_running = runningTime > 0;
    if (m_running) {
        int tmpSeconds = runningTime - m_startedAt.secsTo(QDateTime::currentDateTime());
        if (tmpSeconds > 0) {
            setSeconds(tmpSeconds);
            startTimer();
        } else {
            //TODO: We should notify user about expired timer
            m_running = false;
        }
    } else {
        setSeconds(m_startingSeconds);
        if (m_seconds){
            m_startAction->setEnabled(true);
            m_resetAction->setEnabled(true);
        }
    }
}

void Timer::constraintsEvent(Plasma::Constraints constraints)
{
    if (constraints == Plasma::SizeConstraint) {
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

    //Allow sysadmins to run arbitrary commands on timeout
    bool enableCommandLine = KAuthorized::authorizeKAction("run_command") && KAuthorized::authorizeKAction("shell_access");
    m_runCommand = enableCommandLine;
    ui.runCommandCheckBox->setEnabled(enableCommandLine);
    ui.commandLineEdit->setEnabled(enableCommandLine);

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
    connect(ui.showTitleCheckBox, SIGNAL(toggled(bool)), parent, SLOT(settingsModified()));
    connect(ui.hideSecondsCheckBox, SIGNAL(toggled(bool)), parent, SLOT(settingsModified()));
    connect(ui.showMessageCheckBox, SIGNAL(toggled(bool)), parent, SLOT(settingsModified()));
    connect(ui.runCommandCheckBox, SIGNAL(toggled(bool)), parent, SLOT(settingsModified()));
    connect(ui.titleLineEdit, SIGNAL(userTextChanged(QString)), parent, SLOT(settingsModified()));
    connect(ui.messageLineEdit, SIGNAL(userTextChanged(QString)), parent, SLOT(settingsModified()));
    connect(ui.commandLineEdit, SIGNAL(userTextChanged(QString)), parent, SLOT(settingsModified()));
    connect(predefinedTimersUi.defaulttimers , SIGNAL(changed()), parent, SLOT(settingsModified()));
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
        m_timer.stop();
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

    int seconds = m_seconds % 60;
    int mins = (m_seconds % (60 * 60)) / 60 + (m_hideSeconds && seconds > 0 ? 1 : 0);
    int hours =  m_seconds / (60 * 60);

    const QString suffix = (m_seconds < 60 && m_running) ? "_1" : "";

    m_hoursDigit[0]->setElementID(QString::number(hours / 10) + suffix);
    m_hoursDigit[1]->setElementID(QString::number(hours % 10) + suffix);

    m_separator[0]->setElementID(m_separatorBasename + suffix);

    m_minutesDigit[0]->setElementID(QString::number(mins / 10) + suffix);
    m_minutesDigit[1]->setElementID(QString::number(mins % 10) + suffix);

    if (!m_hideSeconds) {
        m_separator[1]->setElementID(m_separatorBasename + suffix);

        m_secondsDigit[0]->setElementID(QString::number(seconds / 10) + suffix);
        m_secondsDigit[1]->setElementID(QString::number(seconds % 10) + suffix);
    }
}

void Timer::slotCountDone()
{
    if (m_showMessage){
        QPixmap icon = KIconLoader::global()->loadIcon("chronometer", KIconLoader::Dialog);

        KNotification *notification = KNotification::event(KNotification::Notification);
        notification->setPixmap(icon);
        notification->setTitle(i18n("Plasma Timer Applet"));
        notification->setText(m_title->isVisible() ? m_title->text() + " - " + m_message : m_message);
        notification->sendEvent();
    }

    if (m_runCommand && !m_command.isEmpty() && KAuthorized::authorizeKAction("run_command") && KAuthorized::authorizeKAction("shell_access")){
        QStringList args = KShell::splitArgs(m_command);
        QString command = args[0];
        args.removeFirst();
        KToolInvocation::kdeinitExec(command, args);
    }
}

void Timer::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *)
{
    resetTimer();
}

void Timer::saveTimer()
{
    KConfigGroup cg = config();
    cg.writeEntry("running", m_running ? m_seconds : 0);
    cg.writeEntry("startedAt", QDateTime::currentDateTime());
    cg.writeEntry("seconds", 0);

    emit configNeedsSaving();
}

void Timer::startTimer()
{
    m_running = true;
    saveTimer();
    setBlinking(false);
    m_timer.start(1000);

    m_startAction->setEnabled(false);
    m_resetAction->setEnabled(true);
    m_stopAction->setEnabled(true);
}

void Timer::stopTimer()
{
    m_running = false;
    saveTimer();
    setBlinking(true);
    m_timer.stop();

    m_startAction->setEnabled(true);
    m_resetAction->setEnabled(true);
    m_stopAction->setEnabled(false);
}

void Timer::resetTimer()
{
    m_running = false;
    saveTimer();
    setBlinking(false);
    m_timer.stop();

    setSeconds(m_startingSeconds);
    m_startAction->setEnabled(true);
    m_resetAction->setEnabled(false);
    m_stopAction->setEnabled(false);
}

void Timer::mousePressEvent(QGraphicsSceneMouseEvent *)
{
    // does nothing but ensure the event is accepted so we get release events
}

void Timer::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && geometry().contains(event->pos())) {
        if (m_running) {
            stopTimer();
        } else if (m_seconds > 0) {
            startTimer();
        }
    }
}

QList<QAction*> Timer::contextualActions()
{
    lstActionTimer->setEnabled( !m_running );
    return actions;
}

void Timer::startTimerFromAction()
{
    QAction *action = dynamic_cast<QAction*> (sender());
    if (!action || action->property("seconds").type() != QVariant::Int) {
        return;
    }

    m_startingSeconds = action->property("seconds").toInt();
    setSeconds(m_startingSeconds);
    startTimer();
}

void Timer::digitChanged(int value)
{
    if (m_running) {
        return;
    }

    if (value < 0) {
        if (m_seconds >= abs(value)) {
            setSeconds((m_seconds - abs(value)) % 86400);
        }
    } else {
        setSeconds((m_seconds + abs(value)) % 86400);
    }

    if (m_seconds != 0) {
        m_startAction->setEnabled(true);
        m_resetAction->setEnabled(true);
    } else {
        m_startAction->setEnabled(false);
        m_resetAction->setEnabled(false);
    }
}

void Timer::toggleTimerVisible()
{
    if (!m_blinkAnim) {
        QSequentialAnimationGroup *animGroup = new QSequentialAnimationGroup(this);

        QPropertyAnimation *anim = new QPropertyAnimation(this, "digitOpacity", animGroup);
        anim->setDuration(800);
        anim->setEasingCurve(QEasingCurve::InOutQuad);
        anim->setStartValue(1.0);
        anim->setEndValue(0.2);

        animGroup->addAnimation(anim);
        animGroup->addPause(400);

        m_blinkAnim = animGroup;
    }

    m_blinkAnim->setDirection(qFuzzyCompare(m_hoursDigit[0]->opacity(), 1.0) ?
                              QAbstractAnimation::Forward : QAbstractAnimation::Backward);
    m_blinkAnim->start();
}

void Timer::setBlinking(bool blinking)
{
    if (blinking) {
        toggleTimerVisible();
        connect(m_blinkAnim, SIGNAL(finished()), this, SLOT(reverseBlinkAnim()));
    } else if (m_blinkAnim) {
        disconnect(m_blinkAnim, SIGNAL(finished()), this, SLOT(reverseBlinkAnim()));
        m_blinkAnim->setDirection(QAbstractAnimation::Backward);
    }
}

qreal Timer::digitOpacity() const
{
    return m_hoursDigit[0]->opacity();
}

void Timer::setDigitOpacity(qreal opacity)
{
    m_hoursDigit[0]->setOpacity(opacity);
    m_hoursDigit[1]->setOpacity(opacity);

    m_minutesDigit[0]->setOpacity(opacity);
    m_minutesDigit[1]->setOpacity(opacity);

    m_secondsDigit[0]->setOpacity(opacity);
    m_secondsDigit[1]->setOpacity(opacity);

    m_separator[0]->setOpacity(opacity);
    m_separator[1]->setOpacity(opacity);
}

void Timer::reverseBlinkAnim()
{
    m_blinkAnim->setDirection(m_blinkAnim->direction() == QAbstractAnimation::Forward ?
                              QAbstractAnimation::Backward :
                              QAbstractAnimation::Forward);
    if (m_blinkAnim->state() != QAbstractAnimation::Running) {
        m_blinkAnim->start();
    }
}

#include "timer.moc"
