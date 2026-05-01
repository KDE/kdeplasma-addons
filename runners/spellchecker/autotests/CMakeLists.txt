# SPDX-License-Identifier: BSD-3-Clause
# SPDX-FileCopyrightText: 2022 Fushan Wen <qydwhotmail@gmail.com>

remove_definitions(-DQT_NO_CAST_FROM_ASCII)

include(ECMAddTests)

ecm_add_test(spellcheckrunnertest.cpp TEST_NAME spellcheckrunnertest
    LINK_LIBRARIES
    Qt::Test
    krunner_spellcheck_static
)

add_test(NAME kcm_runner_spellcheck_smoketest COMMAND kcmshell6 --smoke-test kf6/krunner/kcms/kcm_krunner_spellcheck)
set_tests_properties(kcm_runner_spellcheck_smoketest PROPERTIES
    ENVIRONMENT_MODIFICATION QT_PLUGIN_PATH=path_list_prepend:${CMAKE_BINARY_DIR}/bin
)
