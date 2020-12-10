/*
 *   SPDX-FileCopyrightText: 2015 Bernhard Friedreich <friesoft@gmail.com>
 *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef TIMER_H
#define TIMER_H

#include <QObject>

class Timer : public QObject
{
    Q_OBJECT

public:
    explicit Timer(QObject *parent = nullptr);
    Q_INVOKABLE void runCommand(const QString &command);
    /*!
     * Represent \a seconds as s string representing time duration with
     * given format based on QTime.
     */
    Q_INVOKABLE QString secondsToString(int seconds, const QString &format);
};

#endif // TIMER_H
