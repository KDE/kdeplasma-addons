/*
 *   Copyright (C) 2009, 2010 Ivan Cukic <ivan.cukic(at)kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License version 2,
 *   or (at your option) any later version, as published by the Free
 *   Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef LANCELOTPARTS_MODELS_PARTSMERGEDMODEL_H
#define LANCELOTPARTS_MODELS_PARTSMERGEDMODEL_H

#include <Lancelot/Models/BaseMergedModel>

#include <KFileItem>

namespace Models {

class PartsMergedModel: public Lancelot::Models::BaseMergedModel {
    Q_OBJECT
public:
    PartsMergedModel();
    ~PartsMergedModel();

    bool append(const QMimeData * mimeData);
    bool append(const QString & mimeData);
    bool append(const QString & path, const KFileItem & fileItem);

    void remove(int index);
    void clear();

    QString serializedData() const;

    bool load(const QString & data);
    bool loadFromFile(const QString & url);
    bool loadFromList(const QStringList & list);
    bool loadDirectory(const QString & url);

    L_Override bool hasModelContextActions(int index) const;
    L_Override void setModelContextActions(int index, Lancelot::PopupMenu * menu);
    L_Override void modelContextActivate(int index, QAction * context);
    L_Override void setModelDropActions(int index,
            Qt::DropActions & actions, Qt::DropAction & defaultAction);
    L_Override void modelDataDropped(int index, Qt::DropAction action);

    L_Override bool dataDropAvailable(int where, const QMimeData * mimeData);
    L_Override void dataDropped(int where, const QMimeData * mimeData);

    L_Override QString selfShortTitle() const;
    L_Override QString selfTitle() const;

private:
    QList < Lancelot::ActionListModel * > m_models;
    QString m_data;

private Q_SLOTS:
    void modelCountUpdated();

Q_SIGNALS:
    void removeModelRequested(int index);
    void modelContentsUpdated();
};

} // namespace Models

#endif // LANCELOTPARTS_MODELS_PARTSMERGEDMODEL_H


