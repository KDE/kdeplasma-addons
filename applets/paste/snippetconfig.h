/*
 * Copyright 2008  Petri Damsten <damu@iki.fi>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef SNIPPETCONFIG_HEADER
#define SNIPPETCONFIG_HEADER

#include <QWidget>
#include "ui_snippetconfig.h"

class QListWidgetItem;
class ConfigData;

class SnippetConfig : public QWidget, public Ui::SnippetConfig
{
    Q_OBJECT
    enum SnippetRoles { TextRole = Qt::UserRole, IconNameRole };
public:
    SnippetConfig(QWidget *parent = 0);
    virtual ~SnippetConfig();

    void getData(ConfigData *data);

public slots:
    void setData(const ConfigData &data);

protected slots:
    void addClicked();
    void removeClicked();
    void addMacroClicked();
    void currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous);
    void nameChanged(const QString& name);
    void textChanged();
    void enableWidgets();
    void iconChanged(const QString &icon);

protected:
    QListWidgetItem *newItem(const QString& text = QString(),
                             const QString& name = i18n("Untitled"));
};

#endif // SNIPPETCONFIG_HEADER
