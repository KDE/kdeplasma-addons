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

#ifndef PREVIEWER_H
#define PREVIEWER_H


#include <QWidget>
#include <QGraphicsLinearLayout>
#include <QGraphicsGridLayout>
#include <QStyleOptionViewItem>

#include <KMimeType>

#include <Plasma/PopupApplet>
#include <Plasma/Svg>

namespace Plasma{
  class Dialog;
  class ScrollBar;
  class PanelSvg;
}
namespace KParts{
   class ReadOnlyPart;
}

class KMenu;
class QGraphicsSceneDragDropEvent;
class QGraphicsWidget;
class KUrl;
class PreviewItemModel;
class PreviewDialog;
class KFileItem;
class PreviewWidget;

namespace KIO{
   class PreviewJob;
}

// here we define the applet
class Previewer : public Plasma::PopupApplet
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.Previewer")

    public:
        Previewer(QObject* parent, const QVariantList &args);
       ~Previewer();

        QList<QAction*> contextualActions();
        void init();

        QGraphicsWidget *graphicsWidget();

    public slots:
        void openFile(QString);
        void goToPage(uint page);
        QString currentFile();
        uint currentPage();

    protected:
        void dropEvent(QGraphicsSceneDragDropEvent *);
        void setupActions();
        void setupPreviewDialog();
        void constraintsEvent(Plasma::Constraints);

    private:
        void addRecent(QAction *action, const QUrl &url);
        void removeRecent(int index);

    private slots:
        virtual void closeFile(bool hide = true);
        virtual void addPreview(const QUrl&, KMimeType::Ptr mimeType = KMimeType::Ptr());
        virtual void reopenPreview();
        virtual void stayOnTop(bool);
        virtual void openFile(KUrl u = KUrl());
        virtual void removeCurrentFromHistory();
        virtual void openUrls(KUrl::List);
        virtual void slotRunClicked();

    private:
       QWidget *m_base;
       Plasma::ScrollBar *m_scrollBar;
       PreviewDialog *m_dialog;
       KParts::ReadOnlyPart *m_part;
       QList<QAction*> m_actions;
       QString m_currentService;
       QString m_currentFile;
       QPoint m_clicked;
       KMenu *m_recents;
       QList<QUrl> m_previewHistory;
       PreviewWidget *m_previewWidget;
};



#endif
