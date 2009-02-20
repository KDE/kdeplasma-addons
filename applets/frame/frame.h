/***************************************************************************
 *   Copyright  2007 by Anne-Marie Mahfouf <annma@kde.org>                 *
 *   Copyright  2007 by Antonio Vinci <mercurio@personellarete.it>         *
 *   Copyright  2008 by Thomas Coopman <thomas.coopman@gmail.com>          *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

#ifndef FRAME_H
#define FRAME_H

#include <QAction>
#include <QPixmap>
#include <QPainter>
#include <QGraphicsItem>
#include <QColor>

#include <plasma/applet.h>
#include <plasma/animator.h>

class ConfigDialog;
class QGraphicsSceneDragDropEvent;
class SlideShow;


class Frame : public Plasma::Applet
{
    Q_OBJECT
public:
    Frame(QObject *parent, const QVariantList &args);
    ~Frame();

    void paintInterface(QPainter *painter, const QStyleOptionGraphicsItem *option,
                        const QRect &contentsRect);
    void init();
    QSizeF contentSizeHint() const;
    virtual QList<QAction*> contextualActions();

public slots:
    void createConfigurationInterface(KConfigDialog *parent);

protected Q_SLOTS:
    void dropEvent(QGraphicsSceneDragDropEvent *event);
    void dragEnterEvent(QGraphicsSceneDragDropEvent *event);
    void configAccepted();
    void updatePicture();
    void slotOpenPicture();

private Q_SLOTS:
    void addDir();
    void removeDir();
    void updateButtons();

protected:
    void constraintsEvent(Plasma::Constraints constraints);
    void updateMenu();

private:
    void stopPotd();
    void initSlideShow();

    /// The current color of the frame
    QColor m_frameColor;
    /// Configuration dialog
    ConfigDialog *m_configDialog;
    /// true if the user wants a frame. If false, there's only the black border around the picture
    bool m_frame;
    /// If true, the frame will have rounded corners
    bool m_roundCorners;
    /// If true, smooth scaling (better visual results, but more cpu intensive) is used for resizing
    bool m_smoothScaling;
    /// If true, the picture will have a drop shadow.
    bool m_shadow;
    /// PoTD
    QString m_potdProvider;
    bool m_potd;
    /// Stores the current picture URL when slideShow is false. Wikipedia Picture of the Day is default.
    KUrl m_currentUrl;
    /// The current slideshow folder
    QStringList m_slideShowPaths;
    unsigned int m_slideNumber;
    int m_slideshowTime;
    /// The size of the current picture
    QSize m_pictureSize;
    /// The action list for the context menu
    QList<QAction*> m_actions;
    QAction* m_openPicture;
    /// Frame & shadow outline thickness
    int m_frameOutline;
    int m_swOutline;
    /// Slideshow
    bool m_slideShow;
    bool m_menuPresent;
    bool m_random;
    bool m_recursiveSlideShow;
    SlideShow* m_mySlideShow;
};

K_EXPORT_PLASMA_APPLET(frame, Frame)

#endif
