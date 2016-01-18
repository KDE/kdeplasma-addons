/*
 *  Copyright 2015  Lars Pontoppidan <dev.larpon@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  2.010-1301, USA.
 */

function loop(object,callback) {
    if(object !== undefined && object !== null) {
        var children = object.children
        for(var i in children) {
            callback(children[i])
            loop(children[i],callback)
        }
    }
}

function remap( old_value, old_min, old_max, new_min, new_max ) {
    // Linear conversion
    // NewValue = (((old_value - old_min) * (new_max - new_min)) / (old_max - old_min)) + new_min
    return (((old_value - old_min) * (new_max - new_min)) / (old_max - old_min)) + new_min
}