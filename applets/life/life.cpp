/***************************************************************************
 *   Copyright 2008 by Davide Bettio <davide.bettio@kdemail.net>           *
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

#include <assert.h>
#include <cstdlib>

#include <QPainter>
#include <QTimer>

#include <Plasma/Svg>
#include <Plasma/Theme>
#include <KDialog>
#include <KDebug>

#include "ui_lifeConfig.h"
#include "life.h"

Life::Life(QObject *parent, const QVariantList &args)
    : Plasma::Applet(parent, args),
    m_dialog(0),
    timer(this)
{
    setHasConfigurationInterface(true);
    resize(128, 128);
}

void Life::init()
{
    KConfigGroup cg = config();

    cellsArrayHeight = cg.readEntry("verticalCells", 64) + 2;
    cellsArrayWidth = cg.readEntry("horizontalCells", 64) + 2;
    stepInterval = cg.readEntry("stepInterval", 1);
    maxGensNumber = cg.readEntry("maxGensNumber", 600);

    qreal left, top, right, bottom;
    getContentsMargins(&left, &top, &right, &bottom);
    setPreferredSize(cellsArrayWidth + left + right,
                     cellsArrayHeight + top + bottom);

    initGame();

    startUpdateTimer();
}

Life::~Life()
{
    delete cells;
    delete nextGenerationCells;
}

void Life::startUpdateTimer()
{
    connect(&timer, SIGNAL(timeout()), this, SLOT(updateGame()));
    timer.start(1000);
}

Qt::Orientations Life::expandingDirections() const
{
    return 0;
}

void Life::updateGame()
{
    if (gensCounter < maxGensNumber) {
        step();
        gensCounter++;
    } else {
        resetGame();
    }

    update();
}

void Life::showConfigurationInterface()
{
    if (m_dialog == 0) {
        m_dialog = new KDialog;
        m_dialog->setCaption( i18nc("@title:window","Configure Life") );
        ui.setupUi(m_dialog->mainWidget());
        m_dialog->mainWidget()->layout()->setMargin(0);
        m_dialog->setButtons( KDialog::Ok | KDialog::Cancel | KDialog::Apply );

        connect( m_dialog, SIGNAL(applyClicked()), this, SLOT(configAccepted()) );
        connect( m_dialog, SIGNAL(okClicked()), this, SLOT(configAccepted()) );
    }

    ui.verticalCells->setValue(cellsArrayHeight - 2);
    ui.horizontalCells->setValue(cellsArrayWidth - 2);
    ui.stepInterval->setValue(stepInterval);
    ui.maxGensNumber->setValue(maxGensNumber);

    QRectF cSize = geometry();
    int maxCells = (cSize.height() < cSize.width()) ? cSize.height() : cSize.width();

    ui.verticalCells->setMaximum(maxCells);
    ui.horizontalCells->setMaximum(maxCells);

    m_dialog->show();
}

void Life::configAccepted()
{
    KConfigGroup cg = config();

    timer.stop();

    int newArrayHeight = ui.verticalCells->value();
    int newArrayWidth = ui.horizontalCells->value();

    if (newArrayHeight != cellsArrayHeight ||
        newArrayWidth != cellsArrayWidth) {
        cellsArrayHeight = ui.verticalCells->value();
        cellsArrayWidth = ui.horizontalCells->value();
        cg.writeEntry("verticalCells", cellsArrayHeight - 2);
        cg.writeEntry("horizontalCells", cellsArrayWidth - 2);

        qreal left, top, right, bottom;
        getContentsMargins(&left, &top, &right, &bottom);
        setPreferredSize(cellsArrayWidth + left + right,
                         cellsArrayHeight + top + bottom);
    }

    stepInterval = ui.stepInterval->value();
    maxGensNumber = ui.maxGensNumber->value();

    cg.writeEntry("stepInterval", stepInterval);
    cg.writeEntry("maxGensNumber", maxGensNumber);

    delete cells;
    delete nextGenerationCells;

    initGame();

    update();

    startUpdateTimer();

    updateConstraints(Plasma::AllConstraints);

    emit configNeedsSaving();
}

void Life::paintInterface(QPainter *p, const QStyleOptionGraphicsItem *option, const QRect &contentsRect)
{
    Q_UNUSED(option)

    int x = contentsRect.x();
    int y = contentsRect.y();
    const int right = contentsRect.width() + contentsRect.x() - 1;
    const int bottom = contentsRect.height() + contentsRect.y() - 1;
    const qreal cellHeight = qMax(1, contentsRect.height() / cellsArrayHeight);
    const qreal cellWidth = qMax(1, contentsRect.width() / cellsArrayWidth);

    for (int i = cellsArrayWidth + 1;
         i < cellsArrayHeight * cellsArrayWidth - cellsArrayWidth;
         i += ((i % cellsArrayWidth) != cellsArrayWidth - 2) ? 1 : 3) {
        if (cells[i]) {
            p->fillRect(x, y, cellWidth, cellHeight, Qt::white);
        }

        bool nextRow = (i % cellsArrayWidth) == cellsArrayWidth - 2;
        if (!nextRow && x + cellWidth > right) {
            i = (((i / cellsArrayWidth) + 1) * cellsArrayWidth) - 2;
            nextRow = true;
        }

        if (nextRow) {
            x = contentsRect.x();
            y += cellHeight;

            if (y + cellHeight > bottom) {
                break;
            }
        } else {
            x += cellWidth;
        }
    }
}

int Life::neighbors(int i)
{
    return cells[i - cellsArrayWidth - 1] + cells[i - cellsArrayWidth] + cells[i - cellsArrayWidth + 1] +
           cells[i - 1] + cells[i + 1] +
           cells[i + cellsArrayWidth - 1] + cells[i + cellsArrayWidth] + cells[i + cellsArrayWidth + 1];
}

void Life::step()
{
	for (int i = cellsArrayWidth + 1; i < cellsArrayHeight * cellsArrayWidth - cellsArrayWidth; i += ((i % cellsArrayWidth) != cellsArrayWidth - 2) ? 1 : 3){
		switch(neighbors(i)){
			case 2:
				nextGenerationCells[i] = cells[i];
		
				break;
		
			case 3:
				nextGenerationCells[i] = 1;
		
				break;
		
			default:
				nextGenerationCells[i] = 0;
	
				break;
		}
	}

	//Cells arrays swap
	char *tmp = cells;
	cells = nextGenerationCells;
	nextGenerationCells = tmp;
}

void Life::initGame()
{
    cells = new char[cellsArrayHeight * cellsArrayWidth];
    nextGenerationCells = new char[cellsArrayHeight * cellsArrayWidth];

    memset(cells, 0, cellsArrayHeight * cellsArrayWidth * sizeof(char));
    memset(nextGenerationCells, 0, cellsArrayHeight * cellsArrayWidth * sizeof(char));

    resetGame();
}

void Life::resetGame()
{
    for (int i = cellsArrayWidth + 1; i < cellsArrayHeight * cellsArrayWidth - cellsArrayWidth; i += ((i % cellsArrayWidth) != cellsArrayWidth - 2) ? 1 : 3){
        cells[i] = rand() % 2;
    }

    gensCounter = 0;
}

#include "life.moc"
