/***************************************************************************
 *   Copyright (C) 2007 by Jesper Thomschutz <jesperht@yahoo.com>          *
 *                         Simon Hausmann <hausmann@kde.org>               *
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

#include <QGraphicsTextItem>
#include <QFileSystemWatcher>
#include <QFile>
#include <QStringList>
#include <QTextDocument>
#include <QGraphicsProxyWidget>

#include <KConfigDialog>

#include <Plasma/Theme>

#include "fileWatcher.h"
#include "fileWatcherConfig.h"

FileWatcher::FileWatcher(QObject *parent, const QVariantList &args)
    : Plasma::Applet(parent, args), config_dialog(0)
{
  setHasConfigurationInterface(true);
  resize(250, 250);
  m_proxy = new QGraphicsProxyWidget(this);
  file = new QFile(this);
  watcher = new QFileSystemWatcher(this);
  textItem = new QGraphicsTextItem(this);
  textItem->setDefaultTextColor(Plasma::Theme::defaultTheme()->color(Plasma::Theme::TextColor));

  textDocument = textItem->document();

  textDocument->setMaximumBlockCount(6);
  textStream = 0;

  configureButton = new QPushButton(i18n("&Configure File Watcher"));
  m_proxy->setWidget( configureButton );
  buttonBox = new QGraphicsLinearLayout(Qt::Vertical, this);
  buttonBox->addItem(m_proxy);

  connect(configureButton, SIGNAL(clicked()), this, SLOT(createConfigurationInterface()));
  configured = false;
  m_proxy->show();
}


FileWatcher::~FileWatcher()
{
  delete textStream;
  textStream = 0;
  textDocument->clear();
  file->close();
  delete m_proxy;
}

void FileWatcher::loadFile(const QString& path)
{
  delete textStream;
  textStream = 0;
  textDocument->clear();

  file->close();
  file->setFileName(path);
  if (!file->open(QIODevice::ReadOnly | QIODevice::Text))
  {
    kDebug() << "Error: could not open file:" << path;
    configured = false;
    configureButton->show();
    return;
  }

  configureButton->hide();
  configured = true;

  textStream = new QTextStream(file);

  newData();

  watcher->removePaths(watcher->files());
  watcher->addPath(path);

  QObject::connect(watcher,SIGNAL(fileChanged(QString)),this,SLOT(newData()));

  QGraphicsItem::setToolTip(path);
}

void FileWatcher::newData()
{
  QTextCursor cursor(textDocument);
  cursor.movePosition(QTextCursor::End);
  cursor.beginEditBlock();

// Slight speed optimization hack for bigger files.
// Doing this is faster than doing unnecessary insertText()
  QString data = textStream->readAll();
  QStringList list = data.split("\n",QString::SkipEmptyParts);

  int rows = list.size() - textDocument->maximumBlockCount();

  if ( rows < 0)
    rows = 0;

  for (int i = rows; i < list.size(); i++)
  {
    cursor.insertText(list.at(i));
    cursor.insertBlock();
  }

  cursor.endEditBlock();
  updateGeometry();
}

QSizeF FileWatcher::contentSizeHint() const
{
  if (!configured)
    return QSizeF(200, 50);

  return textItem->boundingRect().size();
}


void FileWatcher::newPath(const QString& path)
{
  loadFile(path);
}

void FileWatcher::fontColorChanged(const QColor& color)
{
  textItem->setDefaultTextColor(color);
  textItem->update();
}

void FileWatcher::fontChanged(const QFont& font)
{
  textItem->setFont(font);
}

void FileWatcher::maxRowsChanged(int rows)
{
  textDocument->setMaximumBlockCount(rows+1);
  loadFile(file->fileName());
}

void FileWatcher::createConfigurationInterface(KConfigDialog *parent)
{
    config_dialog = new FileWatcherConfig(parent);

    parent->setButtons(  KDialog::Ok | KDialog::Cancel | KDialog::Apply);
    parent->addPage( config_dialog, parent->windowTitle(), icon() );
    parent->setDefaultButton( KDialog::Ok );
    parent->showButtonSeparator( true );

    QObject::connect(config_dialog,SIGNAL(newFile(const QString&)),this,SLOT(newPath(const QString&)));
    QObject::connect(config_dialog,SIGNAL(maxRowsChanged(int)),this,SLOT(maxRowsChanged(int)));
    QObject::connect(config_dialog,SIGNAL(fontChanged(QFont)),this,SLOT(fontChanged(QFont)));
    QObject::connect(config_dialog,SIGNAL(fontColorChanged(QColor)),this,SLOT(fontColorChanged(QColor)));
}
#include "fileWatcher.moc"

