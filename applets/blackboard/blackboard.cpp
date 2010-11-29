/***************************************************************************
 *   Copyright 2009 by Davide Bettio <davide.bettio@kdemail.net>           *
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

#include "blackboard.h"

#include <QGraphicsLinearLayout>

#include <KDebug>
#include <KIcon>

#include <Plasma/Theme>
#include <Plasma/ToolButton>

BlackBoard::BlackBoard(QObject *parent, const QVariantList &args)
    : Plasma::Applet(parent, args),
      blackBoard(0)
{
    setAspectRatioMode(Plasma::IgnoreAspectRatio);
    resize(200, 200);
    setHasConfigurationInterface(false);
    //setBackgroundHints(NoBackground);
}

BlackBoard::~BlackBoard()
{
    if (blackBoard) {
        blackBoard->saveImage();
    }
}

void BlackBoard::init()
{
    QGraphicsLinearLayout *mainLayout = new QGraphicsLinearLayout(Qt::Vertical);
    blackBoard = new BlackBoardWidget(this);
    blackBoard->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));
    mainLayout->addItem(blackBoard);

    buttonsLayout = new QGraphicsLinearLayout(Qt::Horizontal);
    
    addColorButton(QColor(Qt::red));
    addColorButton(QColor(Qt::yellow));
    addColorButton(QColor(Qt::green));
    addColorButton(QColor(Qt::blue));
    addColorButton(Plasma::Theme::defaultTheme()->color(Plasma::Theme::TextColor));

    Plasma::ToolButton *eraseB = new Plasma::ToolButton(this);
    eraseB->setIcon(KIcon(QLatin1String("edit-delete")));
    buttonsLayout->addItem(eraseB);
    connect(eraseB, SIGNAL(clicked()), blackBoard, SLOT(erase())); 
    
    mainLayout->addItem(buttonsLayout);
    
    setLayout(mainLayout);
}

void BlackBoard::addColorButton(QColor color)
{
    QPixmap colorPixmap(22, 22);
    colorPixmap.fill(color);

    Plasma::ToolButton *tB = new Plasma::ToolButton(this);
    tB->setProperty("color", color);
    tB->setIcon(colorPixmap);
    buttonsLayout->addItem(tB);
    connect(tB, SIGNAL(clicked()), this, SLOT(changeColor())); 
}

void BlackBoard::changeColor()
{
    QObject *sender = QObject::sender();
  
    if (!sender || sender->property("color").type() != QVariant::Color) {
        return;
    }

    blackBoard->setBrushColor(sender->property("color").value<QColor>());
}

#include "blackboard.moc"
