// Copyright 2008-2009 by Benoît Jacob <jacob.benoit.1@gmail.com>
// 
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 2 of
// the License or (at your option) version 3 or any later version
// accepted by the membership of KDE e.V. (or its successor approved
// by the membership of KDE e.V.), which shall act as a proxy 
// defined in Section 14 of version 3 of the license.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#ifndef GLOBAL_HEADER
#define GLOBAL_HEADER

#define MAX_PACKET_SIZE 4

#if (defined(__i386__) || defined(_M_IX86)) && !(defined(__x86_64__) || defined(_M_X64))
#define HAVE_PATH_WITH_SSE2_EXPLICTLY_ENABLED
#endif

#define CLAMP(val, min, max) qMin(qMax(val, min), max)

#define MANDELBROT_CENTER_KEY      "mandelbrotcenter"
#define MANDELBROT_ZOOM_KEY        "mandelbrotzoom"
#define MANDELBROT_COLOR1_KEY      "mandelbrotcolor1"
#define MANDELBROT_COLOR2_KEY      "mandelbrotcolor2"
#define MANDELBROT_COLOR3_KEY      "mandelbrotcolor3"
#define MANDELBROT_QUALITY_KEY     "mandelbrotquality"
#define MANDELBROT_LOCK_KEY        "mandelbrotlock"

#define MANDELBROT_QIMAGE_FORMAT   QImage::Format_RGB32

class MandelbrotTile;
class Mandelbrot;
bool system_has_SSE2();

#endif
