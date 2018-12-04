/*
 *   Copyright (C) 2010 Aaron Seigo <aseigo@kde.org>
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

#ifndef DATETIMERUNNER_H
#define DATETIMERUNNER_H

#include <QDateTime>

#include <KRunner/AbstractRunner>
#include <KRunner/QueryMatch>

/**
 * This class looks for matches in the set of .desktop files installed by
 * applications. This way the user can type exactly what they see in the
 * applications menu and have it start the appropriate app. Essentially anything
 * that KService knows about, this runner can launch
 */

class DateTimeRunner : public Plasma::AbstractRunner
{
    Q_OBJECT

public:
    DateTimeRunner(QObject *parent, const QVariantList &args);
    ~DateTimeRunner() override;

    void match(Plasma::RunnerContext &context) override;

private:
    QHash<QString, QDateTime> datetime(const QStringRef &tz);
    void addMatch(const QString &text, const QString &clipboardText,
                  Plasma::RunnerContext &context, const QString& iconName);
};

#endif

