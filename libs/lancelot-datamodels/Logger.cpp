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

#include "Logger.h"
#include <QIODevice>
#include <QFile>
#include <KNotification>
#include <KStandardDirs>
#include <KIconLoader>
#include <KIcon>
#include <KLocalizedString>
#include <QDir>

namespace Lancelot {
namespace Models {

class Logger::Private {
public:
    Private()
        : file(NULL), stream(NULL)
    {
    }

    void openFile();
    void closeFile();

    QIODevice * file;
    QDataStream * stream;
};

Logger * Logger::m_instance = NULL;

Logger::Logger(bool enabled)
    : d(new Private())
{
    setEnabled(enabled);
}

Logger::~Logger()
{
    delete d;
}

Logger * Logger::self(bool enabled)
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

    (*d->stream) << source
             << message
             << timestamp;
}

QString Logger::path()
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

        QObject::connect(notify, SIGNAL(activated(uint)), Logger::self(), SLOT(configureMenu()));

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
        d->openFile();
    } else {
        d->closeFile();
    }
}

bool Logger::isEnabled() const
{
    return d->file != NULL;
}

void Logger::Private::openFile()
{
    if (file) {
        return;
    }

    file = new QFile(Logger::path());
    if (file->open(QIODevice::WriteOnly | QIODevice::Unbuffered | QIODevice::Append)) {
        stream = new QDataStream(file);
        stream->setVersion(QDataStream::Qt_4_0);
    } else {
        KNotification * notify = new KNotification("ErrorOpeningLog");
        notify->setText(i18n("Failed to open the log file. Logging is disabled."));
        notify->setPixmap(KIcon("view-history").pixmap(KIconLoader::SizeMedium, KIconLoader::SizeMedium));
        notify->sendEvent();

        // Show error message
        delete file;
    }
}

void Logger::Private::closeFile()
{
    if (!file) {
        return;
    }

    delete stream;
    file->close();
    delete file;

    file = NULL;
}

} // namespace Models
} // namespace Lancelot
