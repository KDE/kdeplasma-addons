/***************************************************************************
 *   Copyright (C) 2007 by Pino Toscano <pino@kde.org>                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#ifndef KOLOURPICKER_H
#define KOLOURPICKER_H

#include <plasma/applet.h>

#include <qcolor.h>
#include <qhash.h>

class QMenu;
class QWidget;

namespace Plasma
{
    class ToolButton;
} // namespace Plasma

class Kolourpicker : public Plasma::Applet
{
    Q_OBJECT
    public:
        Kolourpicker(QObject *parent, const QVariantList &args);
        ~Kolourpicker();

    public Q_SLOTS:
        virtual void init();

    protected:
        virtual bool sceneEventFilter(QGraphicsItem *watched, QEvent *event);
        virtual bool eventFilter(QObject *watched, QEvent *event);
        virtual QVariant itemChange(GraphicsItemChange change, const QVariant &value);
        void constraintsEvent(Plasma::Constraints constraints);

    private slots:
        void grabClicked();
        void historyClicked();
        void colorActionTriggered(QAction *act);
        void clearHistory(bool save = true);
        void installFilter();

    private:
        void addColor(const QColor &color, bool save = true);
        void saveData(KConfigGroup &cg);

        Plasma::ToolButton *m_grabButton;
        Plasma::ToolButton *m_historyButton;
        QMenu *m_historyMenu;
        QHash<QColor, QAction *> m_menus;
        QStringList m_colors;
        QWidget *m_grabWidget;
};

inline uint qHash(const QColor &color)
{
    return qHash(color.name());
}

K_EXPORT_PLASMA_APPLET(kolourpicker, Kolourpicker)

#endif
