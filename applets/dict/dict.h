/*
 *   Copyright (C) 2008 Nick Shaforostoff <shaforostoff@kde.ru>
 *
 *   based on work by:
 *   Copyright (C) 2007 Thomas Georgiou <TAGeorgiou@gmail.com> and Jeff Cooper <weirdsox11@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU General Public License as
 *   published by the Free Software Foundation; either version 2 of 
 *   the License, or (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef DICT_H
#define DICT_H

#include <Plasma/PopupApplet>
#include <Plasma/DataEngine>

class QTimer;
class CheckableStringListModel;

class QGraphicsProxyWidget;
class QGraphicsLinearLayout;

namespace Plasma
{
    class IconWidget;
    class LineEdit;
    class TextBrowser;
}

class DictApplet: public Plasma::PopupApplet
{
    Q_OBJECT
    public:
        DictApplet(QObject *parent, const QVariantList &args);
        ~DictApplet();
        void init();

        QGraphicsWidget *graphicsWidget();
        void setPath(const QString&);

    public slots:
        void dataUpdated(const QString &name, const Plasma::DataEngine::Data &data);
        void autoDefine(const QString &word);
        void linkDefine(const QString &word);

    protected slots:
        void define();
        void configAccepted();
        void focusEditor();

    protected:
        void createConfigurationInterface(KConfigDialog *parent);
        void popupEvent(bool shown);

    private:
        QString m_source;
        QTimer* m_timer;
        QString m_dataEngine;
        //QGraphicsPixmapItem *m_graphicsIcon;
        QGraphicsWidget *m_graphicsWidget;
        QGraphicsLinearLayout *m_layout;
        QGraphicsLinearLayout *m_horLayout;
        Plasma::LineEdit *m_wordEdit;
        //Plasma::Flash *m_flash;
//         QTextBrowser* m_defBrowser;
        Plasma::TextBrowser* m_defBrowser;
        Plasma::IconWidget *m_icon;

//         QList< QPair<QString, bool> > m_dicts;
        QPointer<CheckableStringListModel> m_dictsModel;
        QStringList m_dicts;
        QHash<QString,bool> m_activeDicts;
};

K_EXPORT_PLASMA_APPLET(qstardict, DictApplet)

#endif
