/*
 *   SPDX-FileCopyrightText: 2011 Matthias Fuchs <mat69@gmx.net>
 *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef CHECK_NEW_STRIPS_H
#define CHECK_NEW_STRIPS_H

#include "engine/comic.h"

/**
 * This class searches for the newest comic strips of predefined comics in a defined interval.
 * Once found it emits lastStrip
 */
class CheckNewStrips : public QObject
{
    Q_OBJECT

public:
    CheckNewStrips(const QStringList &identifiers, ComicEngine *engine, int minutes, QObject *parent = nullptr);

Q_SIGNALS:
    /**
     * @param index of the identifier in identifiers
     * @param identifier of the comic
     * @param suffix of the last comic strip
     * @see CheckNewStrips
     */
    void lastStrip(int index, const QString &identifier, const QString &suffix);

public Q_SLOTS:
    void dataUpdated(const QString &name, const QVariantMap &data);

private Q_SLOTS:
    void start();

private:
    int mMinutes;
    int mIndex;
    ComicEngine *mEngine;
    const QStringList mIdentifiers;
};

#endif
