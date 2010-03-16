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

#include "mix.h"

/** \returns a RGB mix two floating-point colors of which both RGB and HSV representations are known. The coefficient s is in the range [0,1].
  */
Color3 mix(const Color3& a_rgb, const Color3& a_hsv, const Color3& b_rgb, const Color3& b_hsv, qreal s)
{
  if(a_hsv[2] < qreal(0.4) * b_hsv[2]
  || b_hsv[2] < qreal(0.4) * a_hsv[2]
  || a_hsv[1] < qreal(0.4) * b_hsv[1]
  || b_hsv[1] < qreal(0.4) * a_hsv[1])
  {
    // prefer RGB mixing
    return a_rgb*s+b_rgb*(qreal(1)-s);
  }
  else
  {
    // prefer HSV mixing
    Color3 a_hsv_corrected = a_hsv;
    if(a_hsv_corrected[0] - b_hsv[0] > qreal(0.5)) a_hsv_corrected[0] -= qreal(1);
    if(a_hsv_corrected[0] - b_hsv[0] < qreal(-0.5)) a_hsv_corrected[0] += qreal(1);
    Color3 result_hsv = a_hsv_corrected*s + b_hsv*(qreal(1)-s);
    if(result_hsv[0]<qreal(0)) result_hsv[0]+=qreal(1);
    if(result_hsv[0]>qreal(1)) result_hsv[0]-=qreal(1);
    Color3 result_rgb;
    QColor::fromHsvF(result_hsv[0], result_hsv[1], result_hsv[2])
           .getRgbF(&result_rgb[0], &result_rgb[1], &result_rgb[2]);
    return result_rgb;
  }
}

unsigned char qreal_to_uchar_color_channel(qreal t)
{
  qreal t_scaled = qreal(255)*t;
  qreal floor_t_scaled = std::floor(t_scaled);
  qreal probability_to_add_one = t_scaled - floor_t_scaled;
  // note: qrand() is thread-safe, std::rand() isn't.
  qreal result = floor_t_scaled + (qrand() < int(qreal(RAND_MAX)*probability_to_add_one) ? qreal(1) : qreal(0));
  int result_int_clamped = (int) CLAMP(result, qreal(0), qreal(255));
  return (unsigned char) result_int_clamped;
}
