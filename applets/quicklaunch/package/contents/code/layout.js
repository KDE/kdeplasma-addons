/*
 *  Copyright 2015 David Rosca <nowrep@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License as
 *  published by the Free Software Foundation; either version 2 of
 *  the License or (at your option) version 3 or any later version
 *  accepted by the membership of KDE e.V. (or its successor approved
 *  by the membership of KDE e.V.), which shall act as a proxy
 *  defined in Section 14 of version 3 of the license.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>
 */

function itemPadding()
{
    return units.smallSpacing / 2;
}

function rows()
{
    if (vertical) {
        return Math.ceil(grid.count / maxSectionCount);
    }
    return Math.min(grid.count, maxSectionCount);
}

function cols()
{
    if (vertical) {
        return Math.min(grid.count, maxSectionCount);
    }
    return Math.ceil(grid.count / maxSectionCount);
}

function minimumWidth()
{
    var w = cols() * minimumCellWidth();
    if (!vertical && popupArrow.visible) {
        w += popupArrow.width;
    }
    return w;
}

function minimumHeight()
{
    var h = rows() * minimumCellHeight();
    if (vertical && popupArrow.visible) {
        h += popupArrow.height;
    }
    if (title.length) {
        h += titleLabel.height;
    }
    return h;
}

function preferredWidth()
{
    var w = cols() * preferredCellWidth();
    if (horizontal) {
        w = (preferredHeight() / rows()) * cols();
    }
    if (!vertical && popupArrow.visible) {
        w += popupArrow.width;
    }
    return w;
}

function preferredHeight()
{
    var h = rows() * preferredCellHeight();
    if (vertical) {
        h = (preferredWidth() / cols()) * rows();
        if (popupArrow.visible) {
            h += popupArrow.height;
        }
    }
    if (title.length) {
        h += titleLabel.height;
    }
    return h;
}

function minimumCellWidth()
{
    return units.iconSizes.small + 2 * itemPadding();
}

function minimumCellHeight()
{
    var h = units.iconSizes.small + 2 * itemPadding();
    if (showLauncherNames) {
        h += theme.mSize(theme.defaultFont).height * 2;
    }
    return h;
}

function preferredCellWidth()
{
    return Math.floor(grid.width / cols());
}

function preferredCellHeight()
{
    return Math.floor(grid.height / rows());
}

function popupItemWidth()
{
    var fw = theme.mSize(theme.defaultFont).width;
    return Math.max(root.width, units.iconSizes.medium + 20 * fw);
}

function popupItemHeight()
{
    return units.iconSizes.medium + 2 * itemPadding();
}
