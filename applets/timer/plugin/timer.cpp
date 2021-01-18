/*
 *   SPDX-FileCopyrightText: 2015 Bernhard Friedreich <friesoft@gmail.com>
 *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "timer.h"

#include <KProcess>
#include <KShell>
#include <QDateTime>

Timer::Timer(QObject *parent)
    : QObject(parent)
{
}

void Timer::runCommand(const QString &command)
{
    if (!command.isEmpty()) {
        QStringList split = KShell::splitArgs(command);
        const QString program = split.takeFirst();
        QProcess::startDetached(program, split);
    }
}

QString Timer::secondsToString(int seconds, const QString &format)
{
    return QTime::fromMSecsSinceStartOfDay(seconds * 1000).toString(format);
}
