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

#include "Logger.h"
#include <QIODevice>
#include <QFile>
#include <KNotification>
#include <KStandardDirs>
#include <KIconLoader>
#include <KIcon>
#include <KLocalizedString>
#include <QDir>

Logger * Logger::m_instance = NULL;

Logger::Logger(bool enabled)
    : m_file(NULL), m_stream(NULL)
{
    setEnabled(enabled);
}

Logger::~Logger()
{
}

Logger * Logger::instance(bool enabled)
{
    if (!m_instance) {
        m_instance = new Logger(enabled);
    }
    return m_instance;
}

void Logger::close()
{
    m_instance->setEnabled(false);
    delete m_instance;
    m_instance = NULL;
}

void Logger::log(
        const QString & source,
        const QVariant & message)
{
    if (!isEnabled()) {
        return;
    }

    QDateTime timestamp = QDateTime::currentDateTime();

    (*m_stream) << source
             << message
             << timestamp;
}

QString Logger::path() const
{
    QDir dir;
    QString path = KStandardDirs::locateLocal("data", "lancelot", true);
    if (!path.endsWith('/')) {
        path += '/';
    }
    path += "log/";

    if (!dir.exists(path)) {
        if (!dir.mkpath(path)) {
            return QString();
        }

        KNotification * notify = new KNotification("UsageLoggingOn");
        notify->setText(i18n("Usage logging is activated."));
        notify->setPixmap(KIcon("view-history").pixmap(KIconLoader::SizeMedium, KIconLoader::SizeMedium));
        notify->setActions(QStringList(i18n("Configure")));
        connect(notify, SIGNAL(activated(unsigned int)), this, SLOT(configureMenu()));
        notify->sendEvent();
    }

    QDate date = QDate::currentDate();
    path += "log-" + QString::number(date.year())
             + '-' + QString::number(date.month());

    return path;
}

long Logger::size() const
{
    return 0;
}

void Logger::clear()
{

}

void Logger::setEnabled(bool value)
{
    if (value) {
        openFile();
    } else {
        closeFile();
    }
}

bool Logger::isEnabled() const
{
    return m_file != NULL;
}

void Logger::openFile()
{
    if (m_file) {
        return;
    }

    m_file = new QFile(path());
    if (m_file->open(QIODevice::WriteOnly | QIODevice::Unbuffered | QIODevice::Append)) {
        m_stream = new QDataStream(m_file);
        m_stream->setVersion(QDataStream::Qt_4_0);
    } else {
        KNotification * notify = new KNotification("ErrorOpeningLog");
        notify->setText(i18n("Failed to open the log file. Logging is disabled."));
        notify->setPixmap(KIcon("view-history").pixmap(KIconLoader::SizeMedium, KIconLoader::SizeMedium));
        notify->sendEvent();

        // Show error message
        delete m_file;
    }
}

void Logger::closeFile()
{
    if (!m_file) {
        return;
    }

    delete m_stream;
    m_file->close();
    delete m_file;

    m_file = NULL;
}
