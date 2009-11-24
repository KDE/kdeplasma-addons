/*
 *   Copyright 2009 by Sebastian Kügler <sebas@kde.org>
 *   Copyright 2009 by Richard Moore <rich@kde.org>

 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
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

#ifndef KGRAPHICSWEBSLICE_H
#define KGRAPHICSWEBSLICE_H

#include <qgraphicswidget.h>

class QGraphicsSceneResizeEvent;
class QUrl;

/**
 * @class KGraphicsWebSlice plasma/kgraphicswebslice.h
 *
 * @short A QGraphicsWidget displaying a part of a webpage
 *
 * KGraphicsWebSlice provides a graphicswidget that displays a part of a
 * webpage, based on a QRectF inside the webpage or a CSS selector.
 *
 * The KGraphicsWebSlice needs a URL set and either a slice geometry (using
 * setSliceGeometry(QRectF) or directly a QRectF, for web pages missing the
 * semantic information. When both options are given, element() takes
 * precedence.
 */
class KGraphicsWebSlice : public QGraphicsWidget
{
    Q_OBJECT

public:
    KGraphicsWebSlice( QGraphicsWidget *parent=0 );
    ~KGraphicsWebSlice();

    /**
     * Set the URL of the webpage the is being sliced. If you want
     * to use the more flexible CSS selector to render your slice,
     * the URL should point to the frame you're looking for, not to
     * the mainframe of the page. If the page doesn't use frames,
     * just the URL will work of course.
     *
     * @param url The page or frame URL to render
     **/
    void setUrl( const QUrl &url );

    /**
     * Specify the element of the webpage to be rendered. selector can
     * be any CSS that is matched by WebKit's findElement() API. If you
     * want to select a div with the id "mybox", specify "#mybox" here.
     * The first element found will be used.
     *
     * @param selector the name of the group to access
     **/
    void setElement( const QString &selector );

    /**
     * Directly set the geometry of the piece that is cut out of the
     * webpage. This option is only effective when element().isEmpty().
     *
     * @param geo The QRectF to slice out of the webpage
     **/
    void setSliceGeometry( const QRectF geo );

    /**
     * Refresh the position of the slice. This function can be needed if the
     * geometry changes, or anything else that influences painting and
     * rendering of the webpage.
     *
     **/
    void refresh();

    /**
     * Returns the URL of the webpage that is being sliced.
     *
     * @return The URL the webslice comes from
     **/
    QUrl url();

    /**
     * Returns the CSS selector used to identify the element that is being
     * rendered.
     *
     * @return The CSS selector used to identify the element to render as
     * the slice
     **/
    QString element();

    /**
     * Returns the manually set sliceGeometry inside the page. If you are using
     * the CSS selector mechanism, this function is not what you want. Use the
     * sizing mechanism of the widget instead.
     *
     * @return The CSS selector used to identify the element to render as
     * the slice
     **/
    QRectF sliceGeometry();

    /**
     * Returns a pixmap of the selected slice. Which slice to show is determined
     * by setUrl(QUrl) or setSliceGeometry(QRectF).
     *
     * @return A QPixmap of a part of the webpage
     **/
    QPixmap elementPixmap();

    /**
     * Sets the text shown while the widget is loading. You can use HTML here,
     * as it uses the same webview we use to display the slice. This has to be
     * set before calling setUrl(url).
     *
     * @param text The HTML to display
     **/
    void setLoadingText(const QString &html);

signals:
    /**
     * Emitted when the webslice has found out its preferred geometry
     */
    void sizeChanged(QSizeF);
    /**
     * Emitted when the webpage has loaded and the initial slicing is done
     */
    void loadFinished();

protected slots:
    /**
     * Initial creating of slice and setting of sizing information.
     */
    void createSlice();
    /**
     * Reimplemented from QGraphicsWidget
     */
    void resizeEvent ( QGraphicsSceneResizeEvent * event );

private:
    struct KGraphicsWebSlicePrivate *d;
};

#endif // KGRAPHICSWEBSLICE_H

