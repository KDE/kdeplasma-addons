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
};

#endif // TIMER_H
