/*
  Copyright (c) 2007 Paolo Capriotti <p.capriotti@gmail.com>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.
*/

#ifndef BACKGROUNDDELEGATE_H
#define BACKGROUNDDELEGATE_H

#include <QAbstractItemDelegate>

class BackgroundDelegate : public QAbstractItemDelegate
{
public:
    enum {
        AuthorRole = Qt::UserRole,
        ScreenshotRole,
        ResolutionRole
    };
    
    enum {
      SCREENSHOT_SIZE = 128,
      BLUR_INCREMENT = 9,
      MARGIN = 6,
      BLUR_PAD = 6
    };

    BackgroundDelegate(QObject *listener,
                       float ratio, QObject *parent = 0);

    virtual void paint(QPainter *painter,
                       const QStyleOptionViewItem &option,
                       const QModelIndex &index) const;
    virtual QSize sizeHint(const QStyleOptionViewItem &option,
                           const QModelIndex &index) const;

			       void resetMaxHeight() { m_maxHeight = 0; }
    int m_maxHeight;
private:
    int m_maxWidth;
    QObject *m_listener;
    float m_ratio;
};

#endif // BACKGROUNDDELEGATEL_H
