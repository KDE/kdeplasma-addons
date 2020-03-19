/* Copyright 2010  Anton Kreuzkamp <akreuzkamp@web.de>
 * Copyright 2020  Alexander Lohnau <alexander.lohnau@gmx.de>
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

#ifndef CHARRUNNERCONFIG_H
#define CHARRUNNERCONFIG_H

#include "ui_charrunner_config.h"

// KF
#include <KCModule>


class CharacterRunnerConfigForm : public QWidget, public Ui::CharacterRunnerConfigUi
{
  Q_OBJECT

  public:
    explicit CharacterRunnerConfigForm(QWidget* parent);
};

class CharacterRunnerConfig : public KCModule
{
  Q_OBJECT

  public:
    explicit CharacterRunnerConfig(QWidget* parent, const QVariantList& args = QVariantList());

  public Q_SLOTS:
    void save() override;
    void load() override;
    void defaults() override;

  private Q_SLOTS:
    void addItem();
    void deleteItem();
    void validateAddButton();
    void validateDeleteButton();

  private:
    CharacterRunnerConfigForm* m_ui;
};
#endif
