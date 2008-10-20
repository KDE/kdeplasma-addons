/*
 *  Copyright 2008 by Alessandro Diaferia <alediaferia@gmail.com>

 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License as
 *  published by the Free Software Foundation; either version 2 of
 *  the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef PREVIEWDIALOG_H
#define PREVIEWDIALOG_H

#include <QWheelEvent>

// Plasma
#include <plasma/dialog.h>

class QLabel;
class QPixmap;

class PreviewDialog : public Plasma::Dialog
{
    Q_OBJECT

    public:
        PreviewDialog(QWidget *parent = 0);
        //~PreviewDialog();

        QWidget* baseWidget();
        QLabel* titleLabel();
        void setMimeIcon(const QPixmap& );
        void setTitle(const QString& );

    signals:
        void closeClicked();
        void removeClicked();
        void runClicked();

    protected slots:
        void updateColors();

    protected:
        void mouseMoveEvent(QMouseEvent *event);
        void mousePressEvent(QMouseEvent *event);
        void mouseReleaseEvent(QMouseEvent *event);

    private:
        QWidget *b_widget;
        QLabel *m_label;
        QLabel *mime_icon;
        bool isMoving;
        bool isResizing;
        QPoint startPos;

};

#endif
