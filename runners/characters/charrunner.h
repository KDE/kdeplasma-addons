/* Copyright 2010  Anton Kreuzkamp <akreuzkamp@web.de>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) version 3, or any
 * later version accepted by the membership of KDE e.V. (or its
 * successor approved by the membership of KDE e.V.), which shall
 * act as a proxy defined in Section 6 of version 3 of the license.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef CHARRUNNER_H
#define CHARRUNNER_H

#include <Plasma/AbstractRunner>
#include "charrunner_config.h"

class CharacterRunner : public Plasma::AbstractRunner
{
  Q_OBJECT

  public:
    CharacterRunner(QObject* parent, const QVariantList &args);
    ~CharacterRunner();
    void match(Plasma::RunnerContext &context);
    void reloadConfiguration();
	
  private:
    //config-variables
    QString m_triggerWord;
    QList<QString> m_aliases;
    QList<QString> m_codes;
};

K_EXPORT_PLASMA_RUNNER(CharacterRunner, CharacterRunner)

#endif
