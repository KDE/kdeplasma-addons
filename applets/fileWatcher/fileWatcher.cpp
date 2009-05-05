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
#include <KDirWatch>
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
  watcher = new KDirWatch(this);
  textItem = new FileWatcherTextItem(this);
  textItem->moveBy(contentsRect().x(), contentsRect().y());
  textItem->setSize((int) contentsRect().width(), (int) contentsRect().height());
  textDocument = textItem->document();

  QObject::connect(watcher, SIGNAL(dirty(QString)), this, SLOT(loadFile(QString)));
  QObject::connect(watcher, SIGNAL(created(QString)), this, SLOT(loadFile(QString)));
  QObject::connect(watcher, SIGNAL(deleted(QString)), this, SLOT(fileDeleted(QString)));

  KConfigGroup cg = config();

  QString path = cg.readEntry("path", QString());
  textItem->setDefaultTextColor(cg.readEntry("textColor", Plasma::Theme::defaultTheme()->color(Plasma::Theme::TextColor)));
  textItem->setFont(cg.readEntry("font", Plasma::Theme::defaultTheme()->font(Plasma::Theme::DefaultFont)));

  m_filters = cg.readEntry("filters", QStringList());
  m_showOnlyMatches = cg.readEntry("showOnlyMatches", false);
  m_useRegularExpressions = cg.readEntry("useRegularExpressions", false);

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

void FileWatcher::fileDeleted(const QString &path)
{
  delete textStream;
  textStream = 0;
  file->close();
  setConfigurationRequired(true, i18n("Could not open file: %1", path));
  textDocument->clear();
}

void FileWatcher::loadFile(const QString& path)
{
  if (path.isEmpty()) return;

  bool newFile = !textStream || m_currentPath != path;

  if (newFile) {
    delete textStream;
    textStream = 0;
    watcher->removeFile(m_currentPath);
    watcher->addFile(path);
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

    textStream = new QTextStream(file);

    setConfigurationRequired(false);
    QGraphicsItem::setToolTip(path);
    m_currentPath = path;
    textDocument->clear();
  }

  newData();
}

void FileWatcher::newData()
{
  QTextCursor cursor(textDocument);
  cursor.movePosition(QTextCursor::End);
  cursor.beginEditBlock();

  QStringList list;

  {
    //Slight speed optimization hack for bigger files.
    //Doing this is faster than doing unnecessary insertText()
    QString data = textStream->readAll();
    QStringList tmpList = data.split('\n', QString::SkipEmptyParts);
  
    if (m_showOnlyMatches){
            for (int i = tmpList.size() - 1; i >= 0; i--){
                for (int j = 0; j < m_filters.size(); j++){
                    if (tmpList.at(i).contains(QRegExp(m_filters.at(j), Qt::CaseSensitive, m_useRegularExpressions ? QRegExp::RegExp : QRegExp::FixedString))){
                        list.insert(0, tmpList.at(i));
                    }
                }

                if (list.size() == textDocument->maximumBlockCount()) break;
             }
    }else{
        list = tmpList;
    }
  }

  // go through the lines of readed block
  for (int i = 0; i < list.size(); i++){
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

    widget = new QWidget();
    filtersUi.setupUi(widget);
    parent->addPage(widget, i18n("Filters"), icon());

    filtersUi.filtersListBox->setItems(m_filters);
    filtersUi.showOnlyMatchesCheckBox->setChecked(m_showOnlyMatches);
    filtersUi.useRegularExpressionsRadioButton->setChecked(m_useRegularExpressions);

}

void FileWatcher::configAccepted()
{
    KConfigGroup cg = config();

    QFileInfo file(ui.pathUrlRequester->url().toLocalFile());
    QString tmpPath;

    if (file.isFile()){
        tmpPath = file.absoluteFilePath();
        cg.writePathEntry("path", file.absoluteFilePath());
    }

    textItem->setFont(ui.fontRequester->font());
    cg.writeEntry("font", ui.fontRequester->font());

    textItem->setDefaultTextColor(ui.fontColorButton->color());
    cg.writeEntry("textColor", ui.fontColorButton->color());

    m_filters = filtersUi.filtersListBox->items();
    cg.writeEntry("filters", m_filters);

    m_showOnlyMatches = filtersUi.showOnlyMatchesCheckBox->isChecked();
    cg.writeEntry("showOnlyMatches", m_showOnlyMatches);

    m_useRegularExpressions = filtersUi.useRegularExpressionsRadioButton->isChecked();
    cg.writeEntry("useRegularExpressions", m_useRegularExpressions);

    textItem->update();
    loadFile(tmpPath);

    emit configNeedsSaving();
}

#include "fileWatcher.moc"
