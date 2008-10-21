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

#include <Plasma/Theme>

FileWatcher::FileWatcher(QObject *parent, const QVariantList &args)
    : Plasma::Applet(parent, args),
      m_autoResize(false)
{
  setAspectRatioMode(Plasma::IgnoreAspectRatio);
  setHasConfigurationInterface(true);
  resize(400, 400);
}

void FileWatcher::init()
{
  file = new QFile(this);
  watcher = new QFileSystemWatcher(this);
  textItem = new QGraphicsTextItem(this);
  textItem->moveBy(eBorderSize,eBorderSize);
  textDocument = textItem->document();
  textStream = 0;

  KConfigGroup cg = config();

  QString path = cg.readEntry("path", QString());
  textItem->setDefaultTextColor(cg.readEntry("textColor", Plasma::Theme::defaultTheme()->color(Plasma::Theme::TextColor)));
  textItem->setFont(cg.readEntry("font", Plasma::Theme::defaultTheme()->font(Plasma::Theme::DefaultFont)));
  textDocument->setMaximumBlockCount(cg.readEntry("maxRows", 5));
  textItem->update();

  m_autoResize = cg.readEntry("autoResize",false);

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
    setConfigurationRequired(true, i18n("Could not open file: %1", path));
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
  QStringList list = data.split('\n', QString::SkipEmptyParts);

  int rows = list.size() - textDocument->maximumBlockCount();

  if ( rows < 0)
    rows = 0;

  // go throught the lines of readed block
  for (int i = rows; i < list.size(); i++){
    //insert new block befor line, but skip insertion on beginning of document
    //becouse we don't want empty space on first line
    if (cursor.position() != 0){
      cursor.insertBlock();
    }
    cursor.insertText(list.at(i));
  }

  cursor.endEditBlock();

  //if is set flag to resize, resize also plasma widget
  if (m_autoResize == true){
    doAutoResize();
  }

}

void FileWatcher::doAutoResize()
{
  //resize widget only if is document not empty
  if (textItem->document()->isEmpty() == false){
    resize(textItem->boundingRect().width() + eBorderSize*2,textItem->boundingRect().height() + eBorderSize*2);
  }
}

void FileWatcher::createConfigurationInterface(KConfigDialog *parent)
{
    QWidget *widget = new QWidget();
    ui.setupUi(widget);
    parent->setButtons( KDialog::Ok | KDialog::Cancel | KDialog::Apply );
    parent->addPage(widget, i18n("General"), icon());
    connect(parent, SIGNAL(applyClicked()), this, SLOT(configAccepted()));
    connect(parent, SIGNAL(okClicked()), this, SLOT(configAccepted()));

    ui.pathUrlRequester->setPath(file->fileName());
    ui.fontRequester->setFont(m_tmpFont);
    ui.fontColorButton->setColor(textItem->defaultTextColor());
    ui.maxRowsSpinBox->setValue(textDocument->maximumBlockCount());
}

void FileWatcher::configAccepted()
{
    KConfigGroup cg = config();

    QFileInfo file(ui.pathUrlRequester->url().toLocalFile());
    if (file.isFile()){
        m_tmpPath = file.absoluteFilePath();
        cg.writePathEntry("path", file.absoluteFilePath());
    }

    textItem->setFont(ui.fontRequester->font());
    cg.writeEntry("font", ui.fontRequester->font());

    textItem->setDefaultTextColor(ui.fontColorButton->color());
    cg.writeEntry("textColor", ui.fontColorButton->color());

    textDocument->setMaximumBlockCount(ui.maxRowsSpinBox->value());
    cg.writeEntry("maxRows", ui.maxRowsSpinBox->value());

    textItem->update();
    loadFile(m_tmpPath);

    emit configNeedsSaving();
}

#include "fileWatcher.moc"
