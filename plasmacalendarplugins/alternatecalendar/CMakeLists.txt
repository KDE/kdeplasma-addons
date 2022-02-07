# SPDX-FileCopyrightText: 2022 Fushan Wen <qydwhotmail@gmail.com>

# SPDX-License-Identifier: GPL-2.0-or-later

find_package(ICU 66.1 COMPONENTS uc i18n)
set_package_properties(ICU
        PROPERTIES DESCRIPTION "Unicode and Globalization support for software applications"
        TYPE OPTIONAL
        PURPOSE "Provides alternate calendar systems that are not available in QCalendar"
        )
if(ICU_FOUND)
    set(HAVE_ICU TRUE)
endif()

configure_file(config-ICU.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-ICU.h)
