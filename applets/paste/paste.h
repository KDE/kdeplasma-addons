/*
 * Copyright 2008  Petri Damsten <damu@iki.fi>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef PASTE_HEADER
#define PASTE_HEADER

#include <configdata.h>
#include <Plasma/PopupApplet>

class ListForm;
class SnippetConfig;
class AutoPasteConfig;
namespace Plasma {
    class Icon;
}

class Paste : public Plasma::PopupApplet
{
    Q_OBJECT
    public:
        Paste(QObject *parent, const QVariantList &args);
        ~Paste();

        virtual void init();
        virtual void createConfigurationInterface(KConfigDialog *parent);
        virtual QWidget *widget();

    public slots:
        void showOk();
        void configAccepted();
        void resetIcon();
        void configChanged();

    private:
        ListForm *m_list;
        SnippetConfig *m_snippetConfig;
        AutoPasteConfig *m_autoPasteConfig;
        ConfigData cfg;
};

K_EXPORT_PLASMA_APPLET(paste, Paste)

#endif
