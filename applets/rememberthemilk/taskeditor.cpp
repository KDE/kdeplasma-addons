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

#include "taskeditor.h"

#include "taskitem.h"

#include <KComboBox>
#include <KLocale>
#include <KLineEdit>
#include <KPushButton>
#include <KDebug>

#include <QPainter>
#include <QStringList>

#include <Plasma/Theme>
#include <Plasma/Animator>
#include <Plasma/Animation>
#include <Plasma/IconWidget>
#include <Plasma/Service>

TaskEditor::TaskEditor(Plasma::DataEngine* engine, QGraphicsWidget* parent)
  : QGraphicsWidget(parent),
  m_engine(engine),
  m_service(0)
{
  mainLayout = new QGraphicsGridLayout(this);
  nameEdit = new Plasma::LineEdit(this);
  dateEdit = new Plasma::LineEdit(this);
  tagsEdit = new Plasma::LineEdit(this);
  priorityEdit = new Plasma::ComboBox(this);
  completeBox = new Plasma::CheckBox(this);

  nameLabel = new Plasma::Label(this);
  nameLabel->setText(i18n("Name:"));
  dateLabel = new Plasma::Label(this);
  dateLabel->setText(i18n("Due:"));
  tagsLabel = new Plasma::Label(this);
  tagsLabel->setText(i18n("Tags:"));
  priorityLabel = new Plasma::Label(this);
  priorityLabel->setText(i18n("Priority:"));
  completeLabel = new Plasma::Label(this);
  completeLabel->setText(i18n("Complete:"));

  QStringList priorityStrings;
  priorityStrings << i18n("Top Priority") << i18n("Medium Priority") << i18n("Low Priority") << i18n("No Priority");
  priorityEdit->nativeWidget()->addItems(priorityStrings);

  saveChangesButton = new Plasma::PushButton(this);
  connect(saveChangesButton, SIGNAL(clicked()), this, SIGNAL(requestSaveChanges()));
  connect(saveChangesButton, SIGNAL(clicked()), this, SLOT(saveChanges()));
  discardChangesButton = new Plasma::PushButton(this);
  connect(discardChangesButton, SIGNAL(clicked()), this, SIGNAL(requestDiscardChanges()));
  connect(discardChangesButton, SIGNAL(clicked()), this, SLOT(discardChanges()));


  saveChangesButton->setText(i18n("Update Task"));
  saveChangesButton->nativeWidget()->setIcon(KIcon("dialog-ok-apply"));

  discardChangesButton->setText(i18n("Discard Changes"));
  discardChangesButton->nativeWidget()->setIcon(KIcon("dialog-cancel"));

  mainLayout->addItem(nameLabel, 0, 0);
  mainLayout->addItem(nameEdit, 0, 1);
  
  mainLayout->addItem(dateLabel, 1, 0);
  mainLayout->addItem(dateEdit, 1, 1);
  
  mainLayout->addItem(tagsLabel, 2, 0);
  mainLayout->addItem(tagsEdit, 2, 1);

  mainLayout->addItem(priorityLabel, 3, 0);
  mainLayout->addItem(priorityEdit, 3, 1);


  mainLayout->addItem(completeLabel, 4, 0);
  mainLayout->addItem(completeBox, 4, 1);
  
  //mainLayout->setColumnStretchFactor(2, 1);
  mainLayout->setRowStretchFactor(6, 1);

  mainLayout->addItem(saveChangesButton, 7, 0, 1, 2);
  mainLayout->addItem(discardChangesButton, 8, 0, 1, 2);

  setLayout(mainLayout);
}

TaskEditor::~TaskEditor()
{
  // Service is parented to the engine and is thus deleted automatically
}


void TaskEditor::keyPressEvent(QKeyEvent* event) {
  kDebug() << event->key();
  if (event->key() == Qt::Key_Escape) {
    //emit requestDiscardChanges(); // Only works when in line edits... i.e. not good
    //discardChanges();
  }
  QGraphicsItem::keyPressEvent(event);
}


void TaskEditor::setModelIndex(QModelIndex index) {
  m_index = index.data(Qt::RTMTaskIdRole).toULongLong();
  
  m_name = index.data(Qt::RTMNameRole).toString();
  nameEdit->nativeWidget()->clear();
  nameEdit->nativeWidget()->setText(m_name);
  
  m_date = index.data(Qt::RTMDueRole).toDate().toString(Qt::DefaultLocaleShortDate); //FIXME: Allow times within a date
  dateEdit->nativeWidget()->clear();
  dateEdit->nativeWidget()->setText(m_date); 
  
  m_tags = index.data(Qt::RTMTagsRole).toStringList().join(", ");
  tagsEdit->nativeWidget()->clear();
  tagsEdit->nativeWidget()->setText(m_tags);

  priorityEdit->nativeWidget()->setCurrentIndex((index.data(Qt::RTMPriorityRole).toInt()-1) % 4);
  m_priority = priorityEdit->nativeWidget()->currentIndex();
  completeBox->setChecked(index.data(Qt::RTMCompletedRole).toBool());
  
  if (m_service)
    m_service->deleteLater();
  m_service = m_engine->serviceForSource("Task:" + QString::number(m_index));
  connect(m_service, SIGNAL(finished(Plasma::ServiceJob*)), SIGNAL(jobFinished(Plasma::ServiceJob*)));
}

void TaskEditor::discardChanges() {
  startAnimation(fullSize, false);
}

void TaskEditor::saveChanges() {
  if (!m_service)
    return; // No index (and hence no task) has been set, or something is really wrong.
  
  if (m_name != nameEdit->text()) {
    kDebug() << "Name Change: " << nameEdit->text();
    KConfigGroup cg = m_service->operationDescription("setName");
    cg.writeEntry("name", nameEdit->text());
    emit jobStarted(m_service->startOperationCall(cg));
  }

  if (m_date != dateEdit->text()) {
    kDebug() << "Date Change: " << dateEdit->text();
    KConfigGroup cg = m_service->operationDescription("setDueText");
    cg.writeEntry("dueText", dateEdit->text());
    emit jobStarted(m_service->startOperationCall(cg));
  }

  if (m_tags != tagsEdit->text()) {
    QStringList tags = tagsEdit->text().split(',');
    KConfigGroup cg = m_service->operationDescription("setTags");
    cg.writeEntry("tags", tags);
    emit jobStarted(m_service->startOperationCall(cg));
  }

  if (priorityEdit->nativeWidget()->currentIndex() != m_priority) {
    KConfigGroup cg = m_service->operationDescription("setPriority");
    cg.writeEntry("priority", priorityEdit->nativeWidget()->currentIndex() + 1);
    emit jobStarted(m_service->startOperationCall(cg));
  }
  
  if (completeBox->isChecked()) {
    KConfigGroup cg = m_service->operationDescription("setCompleted");
    cg.writeEntry("completed", true);
    emit jobStarted(m_service->startOperationCall(cg));
  }

  startAnimation(fullSize, false);
}


void TaskEditor::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {
  Q_UNUSED(widget)
  QColor wash = Plasma::Theme::defaultTheme()->color(Plasma::Theme::BackgroundColor);
  painter->setBrush(wash);
  painter->setPen(QPen(Qt::NoPen));
  painter->drawRect(option->exposedRect);
}


void TaskEditor::setFullSize(QSizeF size) {
  fullSize = size;
  resize(fullSize);
}


void TaskEditor::startAnimation(QSizeF endSize, bool show) {
    appearing = show;
    if (appearing)
      foreach(QGraphicsItem* child, childItems())
        child->show();

    this->show();

    fullSize = endSize;
    resize(fullSize);

    Plasma::Animation *animation = m_fadeAnimation.data();
    if (!animation) {
      animation = Plasma::Animator::create(Plasma::Animator::FadeAnimation);
      animation->setTargetWidget(this);
      animation->setProperty("startValue", 0.0);
      animation->setProperty("endValue", 1.0);
      animation->setProperty("duration", 100);
      m_fadeAnimation = animation;
      connect(animation, SIGNAL(finished()), this, SLOT(animationFinished()));
    } else if (animation->state() == QAbstractAnimation::Running) {
      animation->pause();
    }

    if (show) {
      animation->setProperty("easingCurve", QEasingCurve::InQuad);
      animation->setProperty("direction", QAbstractAnimation::Forward);
      animation->start(QAbstractAnimation::KeepWhenStopped);
    } else {
      animation->setProperty("easingCurve", QEasingCurve::OutQuad);
      animation->setProperty("direction", QAbstractAnimation::Backward);
      animation->start(QAbstractAnimation::DeleteWhenStopped);
    }
}

void TaskEditor::animationFinished() {
    if (appearing) {
      setPos(0, 0);
      resize(fullSize);
    }
    else {
      hide();
    }

    update();
}

#include "taskeditor.moc"
