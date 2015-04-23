#   Copyright (C) 2007, 2008, 2009, 2010 Ivan Cukic <ivan.cukic(at)kde.org>
#
#   This program is free software; you can redistribute it and/or modify
#   it under the terms of the GNU General Public License version 2,
#   or (at your option) any later version, as published by the Free
#   Software Foundation
#
#   This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU General Public License for more details
#
#   You should have received a copy of the GNU General Public
#   License along with this program; if not, write to the
#   Free Software Foundation, Inc.,
#   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

# cmake macro to see if we have libLancelot

# LANCELOT_INCLUDE_DIR
# LANCELOT_FOUND
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
# 3. Neither the name of the University nor the names of its contributors
#    may be used to endorse or promote products derived from this software
#    without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
# OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
# HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
# LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
# OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
# SUCH DAMAGE.

if (LANCELOT_INCLUDE_DIR AND LANCELOT_LIBS)
   # Already in cache, be silent
   # This probably means that libLancelot is a part
   # of current build or that this script was
   # already invoked
   set(Lancelot_FIND_QUIETLY TRUE)
   message("Lancelot variables already set")
endif (LANCELOT_INCLUDE_DIR AND LANCELOT_LIBS)

if (NOT Lancelot_FIND_QUIETLY)
   message("Searching for Lancelot")

   find_path(LANCELOT_INCLUDE_DIR NAMES lancelot/lancelot.h
      PATHS
      ${KDE4_INCLUDE_DIR}
      ${INCLUDE_INSTALL_DIR}
   )

   find_library(LANCELOT_LIBS NAMES lancelot
      PATHS
      ${KDE4_LIB_DIR}
      ${LIB_INSTALL_DIR}
   )

   include(FindPackageHandleStandardArgs)
   FIND_PACKAGE_HANDLE_STANDARD_ARGS(Lancelot DEFAULT_MSG LANCELOT_LIBS LANCELOT_INCLUDE_DIR )

   mark_as_advanced(LANCELOT_INCLUDE_DIR LANCELOT_LIBS)
endif (NOT Lancelot_FIND_QUIETLY)

message("-- Found Lancelot include dir: ${LANCELOT_INCLUDE_DIR}")
message("-- Found Lancelot lib: ${LANCELOT_LIBS}")
