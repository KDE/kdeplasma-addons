/*
 *   Copyright (C) %{CURRENT_YEAR} by %{AUTHOR} <%{EMAIL}>                      *
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef %{APPNAMEUC}_H
#define %{APPNAMEUC}_H

#include <plasma/potdprovider/potdprovider.h>
// Qt
#include <QImage>

class KJob;

class %{APPNAME} : public PotdProvider
{
    Q_OBJECT

public:
    /**
     * Creates a new %{APPNAME}.
     *
     * @param parent the parent object
     * @param args arguments from the plugin factory (unused)
     */
    %{APPNAME}(QObject *parent, const QVariantList &args);

    /**
     * Destroys the provider.
     */
    ~%{APPNAME}() override;

    /**
     * Returns the requested image.
     */
    QImage image() const override;

private:
    void handleFinishedFeedRequest(KJob *job);
    void handleFinishedImageRequest(KJob *job);

private:
    QImage mImage;
};

#endif
