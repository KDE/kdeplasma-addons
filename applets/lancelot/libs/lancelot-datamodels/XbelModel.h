/*
 *   Copyright (C) 2007, 2008, 2009, 2010 Ivan Cukic <ivan.cukic(at)kde.org>
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

#ifndef LANCELOTAPP_MODELS_XBELMODEL_H
#define LANCELOTAPP_MODELS_XBELMODEL_H

#include <lancelot/lancelot_export.h>

#include "BaseModel.h"
#include <QXmlStreamWriter>

namespace Lancelot {
namespace Models {

/**
 * Model that reads Xbel formated bookmarks files
 */
class LANCELOT_EXPORT XbelModel : public BaseModel {
    Q_OBJECT
public:
    explicit XbelModel(QString file);
    virtual ~XbelModel();

protected Q_SLOTS:
    /**
     * Reloads the bookmarks
     */
    void reload();

protected:
    void load();

private:
    class Private;
    Private * const d;
};

} // namespace Models
} // namespace Lancelot

#endif /* LANCELOTAPP_MODELS_XBELMODEL_H */
