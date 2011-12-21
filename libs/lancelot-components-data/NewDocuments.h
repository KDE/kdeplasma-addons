/*
 *   Copyright (C) 2007, 2008, 2009, 2010, 2011, 2012 Ivan Cukic <ivan.cukic(at)kde.org>
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

#ifndef LANCELOT_DATA_NEWDOCUMENTS_H
#define LANCELOT_DATA_NEWDOCUMENTS_H

#include "FolderModel.h"

/**
 * Model contains links to office applications
 */
class NewDocuments: public FolderModel {
    Q_OBJECT
public:
    NewDocuments();
    virtual ~NewDocuments();

    /**
     * @returns path where the .desktop files are being kept
     */
    static QString path();

};

#endif /* LANCELOT_DATA_NEWDOCUMENTS_H */
