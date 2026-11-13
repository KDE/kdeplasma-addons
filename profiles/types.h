/*
 * SPDX-FileCopyrightText: 2026 Christoph Wolk <cwo.kde@posteo.net>
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#pragma once

#include <QtQml/qqmlregistration.h>

#include "profilesmodel.h"

struct ProfilesModelForeign {
    Q_GADGET
    QML_NAMED_ELEMENT(ProfilesModel)
    QML_FOREIGN(ProfilesModel)
};
