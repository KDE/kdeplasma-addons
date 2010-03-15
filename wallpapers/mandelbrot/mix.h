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

#ifndef MIX_HEADER
#define MIX_HEADER

#include "mandelbrot.h"
#include <Eigen/Core>

typedef Eigen::Matrix<qreal,3,1> Color3;
Color3 mix(const Color3& a_rgb, const Color3& a_hsv, const Color3& b_rgb, const Color3& b_hsv, qreal s);
unsigned char qreal_to_uchar_color_channel(qreal t);

#endif // MIX_HEADER
