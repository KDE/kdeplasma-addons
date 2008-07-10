/***************************************************************************
 *   Copyright (C) 2007 by Jesper Thomschutz <jesperht@yahoo.com>          *
 *                         Simon Hausmann <hausmann@kde.org>               *
 *   Copyright (C) 2008 by Davide Bettio <davide.bettio@kdemail.net>       *
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

#include "fileWatcher.h"

#include <QGraphicsTextItem>
#include <QFileSystemWatcher>
#include <QFile>
#include <QStringList>
#include <QTextDocument>
#include <QTextCursor>

#include <KConfigDialog>
#include <KMessageBox>

#include <Plasma/Theme>

#include "fileWatcherConfig.h"

FileWatcher::FileWatcher(QObject *parent, const QVariantList &args)
    : Plasma::Applet(parent, args),
      config_dialog(0)
{
  setHasConfigurationInterface(true);
  resize(400, 400);
}

void FileWatcher::init()
{
  file = new QFile(this);
  watcher = new QFileSystemWatcher(this);
  textItem = new QGraphicsTextItem(this);
  textDocument = textItem->document();
  textStream = 0;

  KConfigGroup cg = config();

  QString path = cg.readEntry("path", QString());
  textItem->setDefaultTextColor(cg.readEntry("textColor", Plasma::Theme::defaultTheme()->color(Plasma::Theme::TextColor)));
  textItem->setFont(cg.readEntry("font", Plasma::Theme::defaultTheme()->font(Plasma::Theme::DefaultFont)));
  textDocument->setMaximumBlockCount(cg.readEntry("maxRows", 5) + 1);

  textItem->update();

  if (path.isEmpty()) {
      setConfigurationRequired(true, i18n("Select a file to watch."));
  } else {
      loadFile(path);
  }
}


FileWatcher::~FileWatcher()
{
  delete textStream;
}

void FileWatcher::loadFile(const QString& path)
{
  if (path.isEmpty()) return;

  delete textStream;
  textStream = 0;
  textDocument->clear();
  watcher->removePaths(watcher->files());
  file->close();

  file->setFileName(path);
  if (!file->open(QIODevice::ReadOnly | QIODevice::Text)){
    KMessageBox::error(0, i18n("Could not open file: %1", path));
    setConfigurationRequired(true);

    return;
  }

  setConfigurationRequired(false);
  QGraphicsItem::setToolTip(path);

  textStream = new QTextStream(file);

  newData();

  watcher->addPath(path);
  QObject::connect(watcher, SIGNAL(fileChanged(QString)), this, SLOT(newData()));
}

void FileWatcher::newData()
{
  QTextCursor cursor(textDocument);
  cursor.movePosition(QTextCursor::End);
  cursor.beginEditBlock();

  //Slight speed optimization hack for bigger files.
  //Doing this is faster than doing unnecessary insertText()
  QString data = textStream->readAll();
  QStringList list = data.split("\n", QString::SkipEmptyParts);

  int rows = list.size() - textDocument->maximumBlockCount();

  if ( rows < 0)
    rows = 0;

  for (int i = rows; i < list.size(); i++){
    cursor.insertText(list.at(i));
    cursor.insertBlock();
  }

  cursor.endEditBlock();
  updateGeometry();
}

void FileWatcher::newPath(const QString& path)
{
  m_tmpPath = path;
}

void FileWatcher::fontColorChanged(const QColor& color)
{
  m_tmpColor = color;
}

void FileWatcher::fontChanged(const QFont& font)
{
  m_tmpFont = font;
}

void FileWatcher::maxRowsChanged(int rows)
{
  m_tmpMaxRows = rows;
}

void FileWatcher::createConfigurationInterface(KConfigDialog *parent)
{
    config_dialog = new FileWatcherConfig(parent);

    parent->setButtons(KDialog::Ok | KDialog::Cancel | KDialog::Apply);
    parent->addPage(config_dialog, parent->windowTitle(), icon());
    parent->showButtonSeparator(true);
    connect(parent, SIGNAL(applyClicked()), this, SLOT(configAccepted()));
    connect(parent, SIGNAL(okClicked()), this, SLOT(configAccepted()));

    QObject::connect(config_dialog, SIGNAL(newFile(const QString&)), this, SLOT(newPath(const QString&)));
    QObject::connect(config_dialog, SIGNAL(maxRowsChanged(int)), this, SLOT(maxRowsChanged(int)));
    QObject::connect(config_dialog, SIGNAL(fontChanged(QFont)), this, SLOT(fontChanged(QFont)));
    QObject::connect(config_dialog, SIGNAL(fontColorChanged(QColor)), this, SLOT(fontColorChanged(QColor)));

    m_tmpPath = file->fileName();
    config_dialog->setPath(m_tmpPath);
    m_tmpColor = textItem->defaultTextColor();
    config_dialog->setTextColor(m_tmpColor);
    m_tmpFont = textItem->font();
    config_dialog->setFont(m_tmpFont);
    m_tmpMaxRows = textDocument->maximumBlockCount() - 1;
    config_dialog->setMaxRows(m_tmpMaxRows);
}

void FileWatcher::configAccepted()
{
    KConfigGroup cg = config();

    cg.writePathEntry("path", m_tmpPath);

    textItem->setDefaultTextColor(m_tmpColor);
    cg.writeEntry("textColor", m_tmpColor);

    textItem->setFont(m_tmpFont);
    cg.writeEntry("font", m_tmpFont);

    textDocument->setMaximumBlockCount(m_tmpMaxRows + 1);
    cg.writeEntry("maxRows", m_tmpMaxRows);

    textItem->update();
    loadFile(m_tmpPath);

    emit configNeedsSaving();
}

#include "fileWatcher.moc"
