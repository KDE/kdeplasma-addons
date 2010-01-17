/*
 *   Copyright (C) 2009, 2010 Ivan Cukic <ivan.cukic(at)kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Lesser/Library General Public License version 2,
 *   or (at your option) any later version, as published by the Free
 *   Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU Lesser/Library General Public License for more details
 *
 *   You should have received a copy of the GNU Lesser/Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef LANCELOTLOGGER_H
#define LANCELOTLOGGER_H

#include <lancelot/lancelot_export.h>

#include <QObject>
#include <QString>
#include <QDateTime>
#include <QIODevice>
#include <QDataStream>
#include <QVariant>

namespace Lancelot {
namespace Models {

class LANCELOT_EXPORT Logger: public QObject
{
    Q_OBJECT

public:
    /**
     * @returns the pointer to the instance
     * @param enabled sets whether the logger should be
     *     enabled. Useful for first invocation
     */
    static Logger * self(bool enabled = false);

    static void close();

    void log(
        const QString & sender,
        const QVariant & message
    );

    /**
     * @returns the path where the log files are located
     */
    static QString path();

    /**
     * @returns the size of the log in bytes
     */
    long size() const;

    /**
     * Deletes all log files
     */
    void clear();

    /**
     * Sets whether log is enabled
     */
    void setEnabled(bool value);

    /**
     * @returns whether log is enabled
     */
    bool isEnabled() const;

private:
    class Private;
    Private * const d;

    Logger(bool enabled);
    virtual ~Logger();

    static Logger * m_instance;
};

} // namespace Models
} // namespace Lancelot

#endif /* LANCELOTLOGGER_H */
