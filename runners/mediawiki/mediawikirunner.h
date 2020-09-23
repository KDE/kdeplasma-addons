/*
 *   SPDX-FileCopyrightText: 2008 Sebastian Kügler <sebas@kde.org>
 *
 *   SPDX-License-Identifier: LGPL-2.0-or-later
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
