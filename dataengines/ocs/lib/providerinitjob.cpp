/*
    This file is part of KDE.

    Copyright (c) 2009 Eckhart Wörner <ewoerner@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,
    USA.
*/

#include "providerinitjob.h"

#include <QTimer>

#include <KUrl>


using namespace Attica;


class ProviderInitJob::Private {
    public:
        QString m_id;
        Provider m_provider;
        Private(const QString& id) : m_id(id)
        {
        }
};


ProviderInitJob::ProviderInitJob(const QString& id, QObject* parent)
    : KJob(parent), d(new Private(id))
{
}


ProviderInitJob::~ProviderInitJob()
{
    delete d;
}


void ProviderInitJob::start()
{
    QTimer::singleShot(0, this, SLOT(doWork()));
}


void ProviderInitJob::doWork()
{
    if (d->m_id == "opendesktop") {
        d->m_provider = Provider(d->m_id, KUrl("https://api.opendesktop.org/v1/"), "OpenDesktop.org");
    }
    emitResult();
}


Provider ProviderInitJob::provider() const
{
    return d->m_provider;
}


#include "providerinitjob.moc"
