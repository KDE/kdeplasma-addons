/***************************************************************************
 *   Copyright (C) 2007 by Jesper Thomschutz <jesperht@yahoo.com>          *
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

#ifndef FILEWATCHER_H
#define FILEWATCHER_H

#include <QGraphicsTextItem>
#include <QFile>
#include <QQueue>
#include <QFileSystemWatcher>
#include <QTextCursor>
#include <QTextDocument>
#include <QGraphicsLinearLayout>

#include <plasma/applet.h>

#include "fileWatcherConfig.h"

class QPushButton;
class QGraphicsProxyWidget;

class FileWatcher : public Plasma::Applet
{
  Q_OBJECT
  public:
    FileWatcher(QObject *parent, const QVariantList &args);
    ~FileWatcher();

    void init();
    QSizeF contentSizeHint() const;

  private:
    void loadFile(const QString& path);

    QFile *file;
    QFileSystemWatcher *watcher;
    FileWatcherConfig *config_dialog;
    QGraphicsTextItem *textItem;
    QTextStream *textStream;
    QTextDocument *textDocument;
    QPushButton *configureButton;
    QGraphicsLinearLayout *buttonBox;
    bool configured;
    QGraphicsProxyWidget *m_proxy;

  private slots:
    void createConfigurationInterface(KConfigDialog *parent);
    void newData();
    void fontChanged(const QFont& font);
    void fontColorChanged(const QColor& color);
    void newPath(const QString& path);
    void maxRowsChanged(int);
};

K_EXPORT_PLASMA_APPLET(fileWatcher, FileWatcher)

#endif
