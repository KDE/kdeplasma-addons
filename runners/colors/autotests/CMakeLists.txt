# SPDX-License-Identifier: BSD-3-Clause
# SPDX-FileCopyrightText: 2025 Kai Uwe Broulik <kde@broulik.de>

include(ECMAddTests)

ecm_add_test(colorrunnertest.cpp TEST_NAME colorrunnertest LINK_LIBRARIES Qt::Gui Qt::Test KF6::Runner)
krunner_configure_test(colorrunnertest krunner_colors)
