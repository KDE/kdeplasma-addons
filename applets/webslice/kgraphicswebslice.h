/*
 *   Copyright 2009-2011 by Sebastian Kügler <sebas@kde.org>
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

#include <qgraphicswebview.h>

class QGraphicsSceneResizeEvent;
class QUrl;
class QWebFrame;

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
class KGraphicsWebSlice : public QGraphicsWebView
{
    Q_OBJECT

public:
    KGraphicsWebSlice( QGraphicsWidget *parent=0 );
    ~KGraphicsWebSlice();

    virtual void paint (QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget * widget = 0);

    /**
     * Set the URL of the webpage the is being sliced. If you want
     * to use the more flexible CSS selector to render your slice,
     * the URL should point to the frame you're looking for, not to
     * the mainframe of the page. If the page doesn't use frames,
     * just the URL will work of course.
     *
     * @param url The page or frame URL to render
     **/
    void loadSlice(const QUrl &url, const QString &selector = QString());

    /**
     * Specify the element of the webpage to be rendered. selector can
     * be any CSS that is matched by WebKit's findElement() API. If you
     * want to select a div with the id "mybox", specify "#mybox" here.
     * The first element found will be used.
     *
     * @param selector the name of the group to access
     **/
    void setElement(const QString &selector);

    /**
     * Directly set the geometry of the piece that is cut out of the
     * webpage. This option is only effective when element().isEmpty().
     *
     * @param geo The QRectF to slice out of the webpage
     **/
    void setSliceGeometry( const QRectF geo );

    /**
     * Returns the CSS selector used to identify the element that is being
     * rendered.
     *
     * @return The CSS selector used to identify the element to render as
     * the slice
     **/
    QString element() const;

    /**
     * Returns the manually set sliceGeometry inside the page. If you are using
     * the CSS selector mechanism, this function is not what you want. Use the
     * sizing mechanism of the widget instead.
     *
     * @param selector Specify a selector to find the geometry of a specific element
     * @return The geometry of the slice within the page, if selector is empty, the current
     * element is show, if the slice is not found, a zero-size rect will be returned.
     **/
    QRectF sliceGeometry(const QString &selector = QString()) const;

    /**
     * Returns a pixmap of the selected slice. Which slice to show is determined
     * by setUrl(QUrl) or setSliceGeometry(QRectF).
     *
     * @return A QPixmap of a part of the webpage
     **/
    QPixmap elementPixmap() const;

    /**
     * Sets the text shown while the widget is loading. You can use HTML here,
     * as it uses the same webview we use to display the slice. This has to be
     * set before calling setUrl(url).
     *
     * @param text The HTML to display
     **/
    void setLoadingText(const QString &html);

    /**
     * Sets the color of the preview overlay mask, if nothing is set, default to black.
     *
     * @param color The color used to paint the preview mask
     **/
    void setPreviewMaskColor(const QColor &color);

    /**
     * Switches the widget to showing a specific slice.
     *
     * @param color The CSS selector used to identify the element of the webpage to show
     **/
    void showSlice(const QString &selector = QString());

    /**
     * Shows the full page unsliced and zoomed out to fit into the widget.
     **/
    void showPage();

public Q_SLOTS:
    /**
     * Renders a preview for a specific element as overlay over the body of the page.
     *
     **/
    void preview(const QString& = QString());

    /**
     * Refresh the position of the slice. This function can be needed if the
     * geometry changes, or anything else that influences painting and
     * rendering of the webpage.
     *
     **/
    void refresh();

    void zoom(const QRectF &area);

Q_SIGNALS:
    /**
     * Emitted when the webslice has found out its preferred geometry
     */
    void sizeChanged(QSizeF);

protected Q_SLOTS:
    /**
     * Slot to catch loading errors, and initiating showing the slice or preview
     */
    void finishedLoading(bool ok);

    /**
     * Reimplemented from QGraphicsWidget
     */
    void resizeEvent ( QGraphicsSceneResizeEvent * event );

    /**
     * slot compressing resize events.
     */
    void resizeTimeout();

private:
    /**
     * Initial creating of slice and setting of sizing information.
     */
    void setPreviewMode(bool = true);
    QRectF previewGeometry(const QString &selector) const;
    void updateElementCache();
    QRectF mapFromPage(const QRectF &rectOnPage) const;
    struct KGraphicsWebSlicePrivate *d;

};

#endif // KGRAPHICSWEBSLICE_H

