/********************************************************************

Copyright (C) 2008 Ryan P. Bitanga <ryan.bitanga@gmail.com>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*********************************************************************/

#ifndef SPELLCHECKCONFIG_H
#define SPELLCHECKCONFIG_H

#include <KCModule>

#include "ui_spellcheck_config.h"

class SpellCheckConfigForm : public QWidget, public Ui::SpellCheckConfigUi
{
    Q_OBJECT
    public:
        explicit SpellCheckConfigForm(QWidget* parent);
};

class SpellCheckConfig : public KCModule
{
    Q_OBJECT
    public:
        explicit SpellCheckConfig(QWidget* parent = 0, const QVariantList& args = QVariantList());
        ~SpellCheckConfig();

    public slots:
        void save();
        void load();
        void defaults();

        void toggleTriggerWord(int state);

    private:
        SpellCheckConfigForm* m_ui;
};

#endif
