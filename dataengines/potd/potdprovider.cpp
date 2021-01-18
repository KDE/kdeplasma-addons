/*
 *   SPDX-FileCopyrightText: 2007 Tobias Koenig <tokoe@kde.org>
 *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "potdprovider.h"

// Qt
#include <QDate>

class PotdProviderPrivate
{
public:
    QString name;
    QDate date;
    QString identifier;
};

PotdProvider::PotdProvider(QObject *parent, const QVariantList &args)
    : QObject(parent)
    , d(new PotdProviderPrivate)
{
    if (args.count() > 0) {
        d->name = args[0].toString();

        d->identifier = d->name;

        if (args.count() > 1) {
            for (int i = 1; i < args.count(); i++) {
                d->identifier += QStringLiteral(":") + args[i].toString();
                QDate date = QDate::fromString(args[i].toString(), Qt::ISODate);
                if (date.isValid()) {
                    d->date = date;
                }
            }
        }
    } else {
        d->name = QStringLiteral("Unknown");
        d->identifier = d->name;
    }
}

PotdProvider::~PotdProvider()
{
}

QString PotdProvider::name() const
{
    return d->name;
}

QDate PotdProvider::date() const
{
    return d->date.isNull() ? QDate::currentDate() : d->date;
}

bool PotdProvider::isFixedDate() const
{
    return !d->date.isNull();
}

QString PotdProvider::identifier() const
{
    return d->identifier;
}
