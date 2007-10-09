/*
 * Copyright (C) 2007 by Alex Merry <alex.merry@kdemail.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef EBNAPPLET_H
#define EBNAPPLET_H

#include <plasma/applet.h>
#include <plasma/dataengine.h>
#include <plasma/widgets/vboxlayout.h>
#include <plasma/widgets/lineedit.h>
#include <plasma/widgets/pushbutton.h>

#include <QString>
#include <QRectF>

class QPainter;
class QStyleOptionGraphicsItem;

class EbnApplet : public Plasma::Applet
{
    Q_OBJECT
    public:
        EbnApplet(QObject *parent, const QVariantList &args);
        ~EbnApplet();

        void paintInterface(QPainter* painter,
                            const QStyleOptionGraphicsItem* option,
                            const QRect& rect);
        QSizeF contentSizeHint() const;
        void constraintsUpdated(Plasma::Constraints);

    public slots:
        void updated(const QString& name, const Plasma::DataEngine::Data& data);
        /**
         * Deal with links from m_viewEdit
         */
        void doLink(const QString& source);

    private:
        /**
         * Visit a new source
         */
        void go(const QString& source = "/");

        Plasma::VBoxLayout* m_layout;
        /**
         * Displays the results
         */
        Plasma::LineEdit* m_viewEdit;

        /**
         * The source currently displayed
         *
         * We ignore updates from everywhere else
         */
        QString watchSource;
};

K_EXPORT_PLASMA_APPLET(ebn, EbnApplet)

#endif // EBNAPPLET_H
