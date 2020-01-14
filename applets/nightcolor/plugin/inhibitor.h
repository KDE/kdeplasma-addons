/*
 * Copyright 2019 Vlad Zahorodnii <vlad.zahorodnii@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <QObject>

/**
 * The Inhibitor class provides a convenient way to temporarily disable Night Color.
 */
class Inhibitor : public QObject
{
    Q_OBJECT
    /**
     * This property holds a value to indicate the current state of the inhibitor.
     */
    Q_PROPERTY(State state READ state NOTIFY stateChanged)

public:
    explicit Inhibitor(QObject *parent = nullptr);
    ~Inhibitor() override;

    /**
     * This enum type is used to specify the state of the inhibitor.
     */
    enum State {
        Inhibiting,   ///< Night Color is being inhibited.
        Inhibited,    ///< Night Color is inhibited.
        Uninhibiting, ///< Night Color is being uninhibited.
        Uninhibited,  ///< Night Color is uninhibited.
    };
    Q_ENUM(State)

    /**
     * Returns the current state of the inhibitor.
     */
    State state() const;

public Q_SLOTS:
    /**
     * Attempts to temporarily disable Night Color.
     *
     * After calling this method, the inhibitor will enter the Inhibiting state.
     * Eventually, the inhibitor will enter the Inhibited state when the inhibition
     * request has been processed successfully by the Night Color manager.
     *
     * This method does nothing if the inhibitor is in the Inhibited state.
     */
    void inhibit();

    /**
     * Attempts to undo the previous call to inhibit() method.
     *
     * After calling this method, the inhibitor will enter the Uninhibiting state.
     * Eventually, the inhibitor will enter the Uninhibited state when the uninhibition
     * request has been processed by the Night Color manager.
     *
     * This method does nothing if the inhibitor is in the Uninhibited state.
     */
    void uninhibit();

Q_SIGNALS:
    /**
     * Emitted whenever the state of the inhibitor has changed.
     */
    void stateChanged();

private:
    class Private;
    QScopedPointer<Private> d;
};
