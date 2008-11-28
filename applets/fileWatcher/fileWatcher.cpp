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
#include <KMimeType>

#include <Plasma/Theme>

#include "fileWatcherTextItem.h"

FileWatcher::FileWatcher(QObject *parent, const QVariantList &args)
    : Plasma::Applet(parent, args)
{
  setAspectRatioMode(Plasma::IgnoreAspectRatio);
  setHasConfigurationInterface(true);
  setBackgroundHints(TranslucentBackground);
  resize(400, 200);
}

FileWatcher::~FileWatcher()
{
  delete textStream;
}

void FileWatcher::init()
{
  file = new QFile(this);
  textStream = 0;
  watcher = new QFileSystemWatcher(this);
  textItem = new FileWatcherTextItem(this);
  textItem->moveBy(contentsRect().x(), contentsRect().y());
  textItem->setSize((int) contentsRect().width(), (int) contentsRect().height());
  textDocument = textItem->document();

  QObject::connect(watcher, SIGNAL(fileChanged(QString)), this, SLOT(loadFile(QString)));

  KConfigGroup cg = config();

  QString path = cg.readEntry("path", QString());
  textItem->setDefaultTextColor(cg.readEntry("textColor", Plasma::Theme::defaultTheme()->color(Plasma::Theme::TextColor)));
  textItem->setFont(cg.readEntry("font", Plasma::Theme::defaultTheme()->font(Plasma::Theme::DefaultFont)));

  updateRows();

  textItem->update();

  if (path.isEmpty()) {
      setConfigurationRequired(true, i18n("Select a file to watch."));
  } else {
      loadFile(path);
  }
}

void FileWatcher::updateRows()
{
    QFontMetrics metrics(textItem->font());
    textDocument->setMaximumBlockCount((int) (contentsRect().height()) / metrics.height());

    if (textStream){
        textDocument->clear();
        textItem->update();
        textStream->seek(0);
        newData();
    }
}

void FileWatcher::constraintsEvent(Plasma::Constraints constraints)
{
    if (constraints & Plasma::SizeConstraint){
        textItem->setSize((int) contentsRect().width(), (int) contentsRect().height());
        updateRows();
    }
}

void FileWatcher::loadFile(const QString& path)
{
  if (path.isEmpty()) return;

  delete textStream;
  textStream = 0;
  textDocument->clear();
  watcher->removePaths(watcher->files());
  file->close();

  KMimeType::Ptr mimeType = KMimeType::findByFileContent(path);
  if (!(mimeType->is("text/plain") || mimeType->name() == QLatin1String("application/x-zerosize"))) {
    setConfigurationRequired(true, i18n("Cannot watch non-text file: %1", path));
    return;
  }

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

  // go through the lines of readed block
  for (int i = rows; i < list.size(); i++){
    // insert new block before line, but skip insertion on beginning of document
    // because we don't want empty space on first line
    if (cursor.position() != 0){
      cursor.insertBlock();
    }
    cursor.insertText(list.at(i));
  }

  cursor.endEditBlock();
}

void FileWatcher::createConfigurationInterface(KConfigDialog *parent)
{
    QWidget *widget = new QWidget();
    ui.setupUi(widget);
    parent->addPage(widget, i18n("General"), icon());
    connect(parent, SIGNAL(applyClicked()), this, SLOT(configAccepted()));
    connect(parent, SIGNAL(okClicked()), this, SLOT(configAccepted()));

    ui.pathUrlRequester->setPath(file->fileName());
    ui.fontRequester->setFont(textItem->font());
    ui.fontColorButton->setColor(textItem->defaultTextColor());
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

    textItem->update();
    loadFile(m_tmpPath);

    emit configNeedsSaving();
}

#include "fileWatcher.moc"
