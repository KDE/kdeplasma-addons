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
#include <QImage>
#include <QPixmap>
#include <QPainter>
#include <QGraphicsItem>
#include <QColor>
#include <QTimer>

#include <plasma/applet.h>
#include <plasma/animator.h>

class ConfigDialog;
class QGraphicsSceneDragDropEvent;
class SlideShow;

namespace Plasma
{
    class ToolButton;
    class Frame;
}

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
    void configChanged();
    void setImageAsWallpaper();

protected Q_SLOTS:
    void dropEvent(QGraphicsSceneDragDropEvent *event);
    void dragEnterEvent(QGraphicsSceneDragDropEvent *event);
    void configAccepted();
    void updatePicture();
    void nextPicture();
    void previousPicture();
    void slotOpenPicture();

private Q_SLOTS:
    void addDir();
    void removeDir();
    void updateButtons();
    void delayedUpdateSize();
    void scalePictureAndUpdate();
    void imageScaled(const QImage &img);
    void reloadImage();

protected:
    void constraintsEvent(Plasma::Constraints constraints);
    void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);
    QSizeF sizeHint(Qt::SizeHint which, const QSizeF &constraint) const;

private:
    void stopPotd();
    void initSlideShow();
    void checkSlideFrame();
    
    QRect preparePainter(QPainter *p, const QRect &rect, const QFont &font, const QString &text);

    /// The current color of the frame
    QColor m_frameColor;
    /// Configuration dialog
    ConfigDialog *m_configDialog;
    /// true if the user wants a frame. If false, there's only the black border around the picture
    bool m_frame;
    /// If true, the frame will have rounded corners
    bool m_roundCorners;
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
    /// Frame & shadow outline thickness
    int m_frameOutline;
    int m_swOutline;
    /// Slideshow
    bool m_slideShow;
    bool m_menuPresent;
    bool m_random;
    bool m_recursiveSlideShow;
    SlideShow* m_mySlideShow;
    /// Auto update
    int m_autoUpdateIntervall;
    bool m_doAutoUpdate;
    QTimer* m_autoUpdateTimer;
    /// Day Change for PoTD
    QTimer *m_dateChangedTimer;
    QDate m_currentDay;
    QList<QAction *> actions;

    QPixmap m_pixmap;
    QImage m_scaledImage;
    QTimer* m_updateTimer;
    Plasma::ToolButton* m_backButton;
    Plasma::ToolButton* m_nextButton;
    Plasma::Frame* m_slideFrame;
    QTimer m_waitForResize;
};

K_EXPORT_PLASMA_APPLET(frame, Frame)

#endif
