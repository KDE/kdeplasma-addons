/*
 *   Copyright (C) 2009 Ivan Cukic <ivan.cukic+kde@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License version 2,
 *   or (at your option) any later version, as published by the Free
 *   Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef LANCELOTLOGGER_H
#define LANCELOTLOGGER_H

#include <QObject>
#include <QString>
#include <QDateTime>
#include <QIODevice>
#include <QDataStream>
#include <QVariant>

class Logger: public QObject
{
    Q_OBJECT

public:
    /**
     * @returns the pointer to the instance
     * @param enabled sets whether the logger should be
     *     enabled. Useful for first invocation
     */
    static Logger * instance(bool enabled = false);

    static void close();

    void log(
        const QString & sender,
        const QVariant & message
    );

    /**
     * @returns the path where the log files are located
     */
    QString path() const;

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
    void openFile();
    void closeFile();

    static Logger * m_instance;

    QIODevice * m_file;
    QDataStream * m_stream;

    Logger(bool enabled);
    virtual ~Logger();
};

#endif /* LANCELOTLOGGER_H */
