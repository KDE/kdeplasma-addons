/* SPDX-FileCopyrightText: 2010 Anton Kreuzkamp <akreuzkamp@web.de>
 * SPDX-FileCopyrightText: 2020 Alexander Lohnau <alexander.lohnau@gmx.de>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#ifndef CHARRUNNER_H
#define CHARRUNNER_H

#include <KRunner/AbstractRunner>

class CharacterRunner : public Plasma::AbstractRunner
{
  Q_OBJECT

  public:
    CharacterRunner(QObject *parent, const KPluginMetaData &metaData, const QVariantList &args);
    ~CharacterRunner() override;

    void reloadConfiguration() override;
    void match(Plasma::RunnerContext &context) override;
    void run(const Plasma::RunnerContext &context, const Plasma::QueryMatch &match) override;

  private:
    //config-variables
    QString m_triggerWord;
    QList<QString> m_aliases;
    QList<QString> m_codes;
};

#endif
