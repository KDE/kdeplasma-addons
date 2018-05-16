/*
 *   Copyright 2008 Sebastian K?gler <sebas@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
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

#ifndef MEDIAWIKIRUNNER_H
#define MEDIAWIKIRUNNER_H

// KF
#include <KRunner/AbstractRunner>
// Qt
#include <QNetworkConfigurationManager>


class MediaWikiRunner : public Plasma::AbstractRunner
{
    Q_OBJECT

public:
    explicit MediaWikiRunner(QObject *parent, const QVariantList &args);
    ~MediaWikiRunner() override;

    void match(Plasma::RunnerContext &context) override;
    void run(const Plasma::RunnerContext &context, const Plasma::QueryMatch &match) override;

private:
    QString m_iconName;
    QString m_name;
    QString m_comment;
    QUrl m_apiUrl;

    QNetworkConfigurationManager m_networkConfigurationManager;
};

#endif
