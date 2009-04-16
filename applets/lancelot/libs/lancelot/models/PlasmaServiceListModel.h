/*
 *   Copyright (C) 2009 Ivan Cukic <ivan.cukic+kde@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Lesser/Library General Public License version 2,
 *   or (at your option) any later version, as published by the Free
 *   Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU Lesser/Library General Public License for more details
 *
 *   You should have received a copy of the GNU Lesser/Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef LANCELOT_PLASMA_SERVICE_LIST_MODEL_H
#define LANCELOT_PLASMA_SERVICE_LIST_MODEL_H

#include <lancelot/lancelot_export.h>
#include <lancelot/models/ActionListModel.h>

#include <QtCore/QVariant>
#include <Plasma/DataEngine>

namespace Lancelot
{

class LANCELOT_EXPORT PlasmaServiceListModel: public ActionListModel {
    Q_OBJECT

public:
    /**
     * Creates a new instance of PlasmaServiceListModel
     */
    PlasmaServiceListModel(QString dataEngine);

    /**
     * Destroys this PlasmaServiceListModel
     */
    virtual ~PlasmaServiceListModel();

    L_Override QString title(int index) const;
    L_Override QString description(int index) const;
    L_Override QIcon icon(int index) const;
    L_Override bool isCategory(int index) const;

    L_Override int size() const;

    L_Override QString selfTitle() const;
    L_Override QIcon selfIcon() const;
    L_Override void activate(int index);

public Q_SLOTS:
    void dataUpdated(const QString & name,
            const Plasma::DataEngine::Data & data);

private:
    class Private;
    Private * const d;
};

} // namespace Lancelot

#endif /* LANCELOT_PLASMA_SERVICE_LIST_MODEL_H */

