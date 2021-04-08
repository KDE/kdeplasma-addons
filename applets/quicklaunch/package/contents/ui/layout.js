/*
 *  SPDX-FileCopyrightText: 2015 David Rosca <nowrep@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

.import org.kde.plasma.core 2.0 as PlasmaCore

function itemPadding() { return PlasmaCore.Units.smallSpacing / 2; }

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
  return PlasmaCore.Units.iconSizes.small + 2 * itemPadding();
}

function minimumCellHeight()
{
  var h = PlasmaCore.Units.iconSizes.small + 2 * itemPadding();
  if (showLauncherNames) {
    h += PlasmaCore.Theme.mSize(PlasmaCore.Theme.defaultFont).height * 2;
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
  var fw = PlasmaCore.Theme.mSize(PlasmaCore.Theme.defaultFont).width;
  return Math.max(root.width, PlasmaCore.Units.iconSizes.medium + 20 * fw);
}

function popupItemHeight()
{
  return PlasmaCore.Units.iconSizes.medium + 2 * itemPadding();
}
