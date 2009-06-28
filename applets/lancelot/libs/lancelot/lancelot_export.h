/*
 *   Copyright (C) 2007 Ivan Cukic <ivan.cukic+kde@gmail.com>
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

#ifndef LANCELOT_EXPORT_H
#define LANCELOT_EXPORT_H

/* needed for KDE_EXPORT and KDE_IMPORT macros */
#include <kdemacros.h>

#ifndef LANCELOT_EXPORT
# if defined(MAKE_LANCELOT_LIB)
   /* We are building this library */
#  define LANCELOT_EXPORT KDE_EXPORT
# else
   /* We are using this library */
#  define LANCELOT_EXPORT KDE_IMPORT
# endif
#endif

#ifndef LANCELOT_EXPORT_DEPRECATED
# define LANCELOT_EXPORT_DEPRECATED KDE_DEPRECATED LANCELOT_EXPORT
#endif

// Override annotation.
#define L_Override virtual

// Include annotation. Does nothing. Needed by PUCK module generator
#define L_INCLUDE(A)

// Extra data needed by PUCK module generator. Does nothing otherwise
#define L_EXTRA(A)

#endif // LANCELOT_EXPORT_H

