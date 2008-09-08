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

#include <Plasma/Applet>

class QFile;
class QFileSystemWatcher;
class QTextDocument;
class QGraphicsTextItem;


class FileWatcher : public Plasma::Applet
{
  Q_OBJECT

  public:
    FileWatcher(QObject *parent, const QVariantList &args);
    ~FileWatcher();

    void init();

  protected slots:
    void configAccepted();

  private slots:
    void newData();

    void createConfigurationInterface(KConfigDialog *parent);

    void fontChanged(const QFont& font);
    void fontColorChanged(const QColor& color);
    void newPath(const QString& path);
    void maxRowsChanged(int);
    void autoResizeChanged(int state);

  private:
    void loadFile(const QString& path);
    //resize plasma applet to textItem size and use eBorderSize for borders
    void doAutoResize();

    QFile *file;
    QFileSystemWatcher *watcher;
    QGraphicsTextItem *textItem;
    QTextStream *textStream;
    QTextDocument *textDocument;

    QColor m_tmpColor;
    QFont m_tmpFont;
    QString m_tmpPath;
    int m_tmpMaxRows;
    bool m_tmpAutoResize;

    bool m_autoResize;
    //distance between text and plasma widget
    enum { eBorderSize = 5 };
};

K_EXPORT_PLASMA_APPLET(fileWatcher, FileWatcher)

#endif
