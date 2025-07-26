/*
    SPDX-FileCopyrightText: 2015 Bernhard Friedreich <friesoft@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include <QProcess>
#include <QTime>
#include <qqmlregistration.h>

class Timer : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

public:
    Q_INVOKABLE void runCommand(const QString &command)
    {
        if (!command.isEmpty()) {
            QStringList split = QProcess::splitCommand(command);
            const QString program = split.takeFirst();
            QProcess::startDetached(program, split);
        }
    }
    /*!
     * Represent \a seconds as s string representing time duration with
     * given format based on QTime.
     */
    Q_INVOKABLE QString secondsToString(int seconds, const QString &format)
    {
        return QTime::fromMSecsSinceStartOfDay(seconds * 1000).toString(format);
    }
};
