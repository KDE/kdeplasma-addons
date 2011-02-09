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

#include "life.h"

#include <assert.h>
#include <cstdlib>

#include <QPainter>

#include <Plasma/Svg>
#include <Plasma/Theme>
#include <KConfigDialog>
#include <KDebug>

Life::Life(QObject *parent, const QVariantList &args)
    : Plasma::Applet(parent, args),
    timer(this)
{
    setHasConfigurationInterface(true);
}

void Life::init()
{
    configChanged();
}

Life::~Life()
{
    delete m_cells;
    delete m_nextGenerationCells;
}

void Life::startUpdateTimer()
{
    connect(&timer, SIGNAL(timeout()), this, SLOT(updateGame()));
    timer.start( m_stepInterval * 1000 );
}

void Life::updateGame()
{
    if (m_gensCounter < m_maxGensNumber) {
        step();
        m_gensCounter++;
    } else {
        resetGame();
    }

    update();
}

void Life::createConfigurationInterface(KConfigDialog *parent)
{
    QWidget *widget = new QWidget();
    ui.setupUi(widget);
    connect(parent, SIGNAL(applyClicked()), this, SLOT(configAccepted()));
    connect(parent, SIGNAL(okClicked()), this, SLOT(configAccepted()));
    parent->addPage(widget, parent->windowTitle(), icon());

    ui.verticalCells->setValue(m_cellsArrayHeight);
    ui.horizontalCells->setValue(m_cellsArrayWidth);
    ui.stepInterval->setValue(m_stepInterval);
    ui.maxGensNumber->setValue(m_maxGensNumber);
    ui.stepInterval->setSuffix(ki18np(" second", " seconds"));
    ui.maxGensNumber->setSuffix(ki18np(" generation", " generations"));
    ui.vertReflectCheckbox->setChecked(m_reflectVertical);
    ui.horizReflectCheckbox->setChecked(m_reflectHorizontal);
    ui.popDensityNumber->setValue(m_popDensityNumber);

    QRectF cSize = geometry();
    const int maxCells = (cSize.height() < cSize.width()) ? cSize.height() : cSize.width();

    ui.verticalCells->setMaximum(maxCells);
    ui.horizontalCells->setMaximum(maxCells);

    connect(ui.horizontalCells, SIGNAL(valueChanged(int)), parent, SLOT(settingsModified()));
    connect(ui.verticalCells, SIGNAL(valueChanged(int)), parent, SLOT(settingsModified()));
    connect(ui.horizReflectCheckbox, SIGNAL(stateChanged(int)), parent, SLOT(settingsModified()));
    connect(ui.vertReflectCheckbox, SIGNAL(stateChanged(int)), parent, SLOT(settingsModified()));
    connect(ui.popDensityNumber, SIGNAL(valueChanged(int)), parent, SLOT(settingsModified()));
    connect(ui.stepInterval, SIGNAL(valueChanged(int)), parent, SLOT(settingsModified()));
    connect(ui.maxGensNumber, SIGNAL(valueChanged(int)), parent, SLOT(settingsModified()));
}

void Life::configChanged()
{
    KConfigGroup cg = config();

    m_cellsArrayHeight = cg.readEntry("verticalCells", 64);
    m_cellsArrayWidth = cg.readEntry("horizontalCells", 64);
    m_stepInterval = cg.readEntry("stepInterval", 1);
    m_maxGensNumber = cg.readEntry("maxGensNumber", 600);
    m_reflectVertical = cg.readEntry("vertReflectCheckbox", false);
    m_reflectHorizontal = cg.readEntry("horizReflectCheckbox", false);
    m_popDensityNumber = cg.readEntry("popDensityNumber", 50);

    qreal left, top, right, bottom;
    getContentsMargins(&left, &top, &right, &bottom);
    setPreferredSize(m_cellsArrayWidth + left + right,
                     m_cellsArrayHeight + top + bottom);
    setMinimumSize(m_cellsArrayWidth + left + right, m_cellsArrayHeight + top + bottom);

    initGame();

    startUpdateTimer();
}

void Life::configAccepted()
{
    KConfigGroup cg = config();

    timer.stop();

    int newArrayHeight = ui.verticalCells->value();
    int newArrayWidth = ui.horizontalCells->value();

    if (newArrayHeight != m_cellsArrayHeight ||
        newArrayWidth != m_cellsArrayWidth) {
        m_cellsArrayHeight = ui.verticalCells->value();
        m_cellsArrayWidth = ui.horizontalCells->value();
        cg.writeEntry("verticalCells", m_cellsArrayHeight);
        cg.writeEntry("horizontalCells", m_cellsArrayWidth);

        qreal left, top, right, bottom;
        getContentsMargins(&left, &top, &right, &bottom);
        setPreferredSize(m_cellsArrayWidth + left + right,
                         m_cellsArrayHeight + top + bottom);
        setMinimumSize(m_cellsArrayWidth + left + right, m_cellsArrayHeight + top + bottom);
    }

    m_stepInterval = ui.stepInterval->value();
    m_maxGensNumber = ui.maxGensNumber->value();
    m_reflectVertical = ui.vertReflectCheckbox->isChecked();
    m_reflectHorizontal = ui.horizReflectCheckbox->isChecked();
    m_popDensityNumber = ui.popDensityNumber->value();

    cg.writeEntry("stepInterval", m_stepInterval);
    cg.writeEntry("maxGensNumber", m_maxGensNumber);
    cg.writeEntry("vertReflectCheckbox", m_reflectVertical);
    cg.writeEntry("horizReflectCheckbox", m_reflectHorizontal);
    cg.writeEntry("popDensityNumber", m_popDensityNumber);

    delete m_cells;
    delete m_nextGenerationCells;

    emit configNeedsSaving();
}

void Life::paintInterface(QPainter *p, const QStyleOptionGraphicsItem *option, const QRect &contentsRect)
{
    Q_UNUSED(option)

    const int cellHeight = (int) qMax(1, contentsRect.height() / m_cellsArrayHeight);
    const int cellWidth = (int) qMax(1, contentsRect.width() / m_cellsArrayWidth);
    int y = contentsRect.y() + (contentsRect.height() - cellHeight * m_cellsArrayHeight) / 2;
    const int x = contentsRect.x() + (contentsRect.width() - cellWidth * m_cellsArrayWidth) / 2;

    int k = 0;
    int x1 = x;
    for (int i = 0; i < m_cellsArrayHeight; i++){
        for (int j = 0; j < m_cellsArrayWidth; j++){
            if (m_cells[k]) {
                p->fillRect(x1, y, cellWidth, cellHeight, Plasma::Theme::defaultTheme()->color(Plasma::Theme::TextColor));
            }
            k++;
            x1 += cellWidth;
        }

        x1 = x;
        y += cellHeight;
    }
}

int Life::neighbors(int i)
{
    int neighbors = 0;
    if (!((i % m_cellsArrayWidth) == 0)) // Not on the left edge, safe to check '-1's
    {
        neighbors += isAlive((i - m_cellsArrayWidth) - 1) + isAlive(i - 1)
            + isAlive((i + m_cellsArrayWidth) - 1);
    }
    if (!((i % m_cellsArrayWidth) == (m_cellsArrayWidth - 1))) // Not on the right edge, safe to check '+1's
    {
        neighbors += isAlive((i - m_cellsArrayWidth) + 1) + isAlive(i + 1)
            + isAlive((i + m_cellsArrayWidth) + 1);
    }

    return neighbors + isAlive(i - m_cellsArrayWidth)
        + isAlive(i + m_cellsArrayWidth);
}

int Life::isAlive(int i)
{
    if ((i < 0) || (i >= (m_cellsArrayHeight * m_cellsArrayWidth))) // Out of bounds
        return 0;
    return m_cells[i];
}

void Life::step()
{
    for (int i = 0; i < (m_cellsArrayHeight * m_cellsArrayWidth); i++){
		switch(neighbors(i)){
			case 2:
				m_nextGenerationCells[i] = m_cells[i];
				break;

			case 3:
				m_nextGenerationCells[i] = 1;
				break;

			default:
				m_nextGenerationCells[i] = 0;
				break;
		}
	}

	//Cells arrays swap
	char *tmp = m_cells;
	m_cells = m_nextGenerationCells;
	m_nextGenerationCells = tmp;
}

void Life::initGame()
{
    m_cells = new char[m_cellsArrayHeight * m_cellsArrayWidth];
    m_nextGenerationCells = new char[m_cellsArrayHeight * m_cellsArrayWidth];

    memset(m_cells, 0, m_cellsArrayHeight * m_cellsArrayWidth * sizeof(char));
    memset(m_nextGenerationCells, 0, m_cellsArrayHeight * m_cellsArrayWidth * sizeof(char));

    resetGame();
}

void Life::resetGame()
{
    for (int i = 0; i < (m_cellsArrayHeight * m_cellsArrayWidth); i++){
        m_cells[i] = (rand() % 100) < m_popDensityNumber ? 1 : 0;
    }

    if (m_reflectHorizontal){
        int hMP = m_cellsArrayHeight / 2;
        for (int i = 0; i < hMP; i++){
            for (int j = 0; j < m_cellsArrayWidth; j++){
                int currValue = m_cells[(i * m_cellsArrayWidth) + j];
                m_cells[(((m_cellsArrayHeight - 1) - i) * m_cellsArrayWidth) + j] = currValue;
            }
        }
    }

    if (m_reflectVertical){
        int wMP = m_cellsArrayWidth / 2;
        for (int i = 0; i < m_cellsArrayHeight; i++){
            for (int j = 0; j < wMP; j++){
                int currValue = m_cells[(i * m_cellsArrayWidth) + j];
                m_cells[(i * m_cellsArrayWidth) + ((m_cellsArrayWidth - 1) - j)] = currValue;
            }
        }
    }

    m_gensCounter = 0;
}

#include "life.moc"
