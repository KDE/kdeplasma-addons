/*
 *   Copyright 2009 Andrew Stromme <astromme@chatonka.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef TASKEDITOR_H
#define TASKEDITOR_H


namespace Plasma {
  class ServiceJob;
  class Animation;
}

//#include <Plasma/Frame>
#include <Plasma/LineEdit>
#include <Plasma/ComboBox>
#include <Plasma/Label>
#include <Plasma/PushButton>
#include <Plasma/CheckBox>

#include <QGraphicsGridLayout>
#include <QGraphicsWidget>
#include <QModelIndex>
#include <QWeakPointer>

class TaskEditor : public QGraphicsWidget
{
Q_OBJECT
public:
  explicit TaskEditor(Plasma::DataEngine *engine, QGraphicsWidget *parent = 0);
  ~TaskEditor();

  void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget);
  void setFullSize(QSizeF size);
  void setModelIndex(QModelIndex index);

protected:
  void keyPressEvent(QKeyEvent* event);
  
signals:
  void requestSaveChanges();
  void requestDiscardChanges();
  void jobStarted(Plasma::ServiceJob *job);
  void jobFinished(Plasma::ServiceJob *job);

public slots:
  void startAnimation(QSizeF endSize, bool show = true);
  void animationFinished();

protected slots:
  void saveChanges();
  void discardChanges();

private:
  Plasma::DataEngine *m_engine;
  qulonglong m_index;
  
  QGraphicsGridLayout *mainLayout;
  Plasma::Label *nameLabel;
  Plasma::LineEdit *nameEdit;
  
  Plasma::Label *dateLabel;
  Plasma::LineEdit *dateEdit;

  Plasma::Label *tagsLabel;
  Plasma::LineEdit *tagsEdit;

  Plasma::Label *priorityLabel;
  Plasma::ComboBox *priorityEdit;

  Plasma::Label *completeLabel;
  Plasma::CheckBox *completeBox;
  
  Plasma::PushButton *saveChangesButton;
  Plasma::PushButton *discardChangesButton;

  bool appearing; // used to know which direction to go in for the animation
  QSizeF fullSize;
  QString m_name;
  QString m_date;
  QString m_tags;
  int m_priority;
  Plasma::Service* m_service;
  QWeakPointer<Plasma::Animation> m_fadeAnimation;
};

#endif // TASKEDITOR_H
