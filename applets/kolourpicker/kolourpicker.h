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
#include <QString>
#include <QColor>

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
        void configChanged();

    protected:
        virtual bool eventFilter(QObject *watched, QEvent *event);
        virtual QVariant itemChange(GraphicsItemChange change, const QVariant &value);
        void constraintsEvent(Plasma::Constraints constraints);

    private slots:
        void grabClicked();
        void historyClicked();
        void colorActionTriggered(QAction *act);
        void colorActionTriggered(const QColor& color);
        void clearHistory(bool save = true);
        void installFilter();
        void setDefaultColorFormat(QAction* act);

    private:
        void addColor(const QColor &color, bool save = true);
        void saveData(KConfigGroup &cg);
        QString toLatex(const QColor& color);
        Plasma::ToolButton *m_grabButton;
        Plasma::ToolButton *m_configAndHistory;
        QMenu *m_configAndHistoryMenu;
        QHash<QColor, QAction *> m_menus;
        QStringList m_colors;
        QStringList m_colors_format;
        QWidget *m_grabWidget;
        QString m_color_format;
};

inline uint qHash(const QColor &color)
{
    return qHash(color.name());
}

K_EXPORT_PLASMA_APPLET(kolourpicker, Kolourpicker)

#endif
