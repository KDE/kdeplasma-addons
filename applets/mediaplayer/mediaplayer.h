/***************************************************************************
 *   Copyright 2008 by Marco Martin <notmart@gmail.com>                    *
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

#ifndef MEDIAPLAYER_HEADER
#define MEDIAPLAYER_HEADER

#include <Plasma/Applet>

#include <phonon/mediaobject.h>

class QGraphicsSceneDragDropEvent;
class QGraphicsLinearLayout;
class QTimer;

namespace Phonon
{
    class VideoWidget;
}

namespace Plasma
{
    class VideoWidget;
}



class MediaPlayer : public Plasma::Applet
{
    Q_OBJECT

    Q_PROPERTY(bool ControlsVisible READ ControlsVisible WRITE SetControlsVisible)

public:
    MediaPlayer(QObject *parent, const QVariantList &args);
    ~MediaPlayer();

    void init();
    void constraintsEvent(Plasma::Constraints constraints);
    void SetControlsVisible(bool visible);
    bool ControlsVisible() const;

protected:
    void dropEvent(QGraphicsSceneDragDropEvent *event);
    void keyPressEvent(QKeyEvent *event);
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);

public Q_SLOTS:
    void PlayPause();
    void ToggleControlsVisibility();
    void RaiseLower();
    void Raise();
    void Lower();
    void SetPosition(int progress);
    void SetVolume(int value);
    void ShowOpenFileDialog();
    void OpenUrl(const QString &path);

private Q_SLOTS:
    void hideControls();

private:
    QGraphicsLinearLayout *m_layout;
    QString m_currentUrl;
    Plasma::VideoWidget *m_video;
    QTimer *m_hideTimer;

    bool m_ticking;
    bool m_raised;
};

#endif
