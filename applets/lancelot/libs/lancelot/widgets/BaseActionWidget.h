/*
 *   Copyright (C) 2007 Ivan Cukic <ivan.cukic+kde@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Lesser/Library General Public License version 2,
 *   or (at your option) any later version, as published by the Free
 *   Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU Lesser/Library General Public License for more details
 *
 *   You should have received a copy of the GNU Lesser/Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

// TODO: Convert to dptr

#ifndef LANCELOT_BASEACTIONWIDGET_H_
#define LANCELOT_BASEACTIONWIDGET_H_

#include "../lancelot_export.h"

#include <QtGui>
#include <QtCore>
#include <plasma/svg.h>
#include "Widget.h"

namespace Lancelot
{

class LANCELOT_EXPORT BaseActionWidget: public Widget {
    Q_OBJECT

public:
    enum InnerOrientation { Vertical, Horizontal };

    BaseActionWidget(QString name = QString(), QString title = QString(),
            QString description = QString(), QGraphicsItem * parent = 0);
    BaseActionWidget(QString name, QIcon * icon, QString title = QString(),
            QString description = QString(), QGraphicsItem * parent = 0);
    BaseActionWidget(QString name, Plasma::Svg * icon, QString title = QString(),
            QString description = QString(), QGraphicsItem * parent = 0);

    virtual ~BaseActionWidget();

    virtual void paintWidget (QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget = 0);

    void setIconSize(QSize size);
    QSize iconSize() const;

    void setIcon(QIcon * icon);
    QIcon * icon() const;

    void setInnerOrientation(InnerOrientation position);
    InnerOrientation innerOrientation() const;

    void setIconInSvg(Plasma::Svg * icon);
    Plasma::Svg * iconInSvg() const;

    void setTitle(const QString & title);
    QString title() const;

    void setDescription(const QString & description);
    QString description() const;

    void setAlignment(Qt::Alignment alignment);
    Qt::Alignment alignment() const;

protected:

    void paintForeground (QPainter * painter);

    QIcon * m_icon;
    Plasma::Svg * m_iconInSvg;
    QSize m_iconSize;
    InnerOrientation m_innerOrientation;

    Qt::Alignment m_alignment;

    QString m_title;
    QString m_description;

private:
    void init();


};

} // namespace Lancelot

#endif /*LANCELOT_BASEACTIONWIDGET_H_*/
