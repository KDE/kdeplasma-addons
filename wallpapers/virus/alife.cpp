/*
  Copyright (c) 2009 by Beat Wolf <asraniel@fryx.ch>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.
*/

#include "alife.h" 

#include <KDebug>

#define VIRUS_GENOME_SIZE 30
#define MAX_AGE 10

Alife::Alife(){
    m_cells = 0;
    m_height = 0;
    m_width = 0;
    m_max_attended = false;
}

Alife::~Alife(){
    for(int i = 0; i < m_livingCells.size(); i++) {
        struct cell* cell = m_livingCells.at(i);
        delete [] cell->code;
    }

    delete []m_cells[0];
    delete []m_cells;
}

void Alife::initVirus()
{
    //magic 2d array code
    if(!m_cells) {
        m_cells = new struct cell*[m_width];
        m_cells[0] = new struct cell[m_width * m_height];
        for(int i = 1; i < m_width; ++i) {
            m_cells[i] = m_cells[i-1] + m_height;
        }
    }
    m_livingCells.clear();

    m_startViruses = 20;

    for(int x = 0; x < m_width; x++) {
        for(int y = 0; y < m_height; y++) {
            struct cell *temp = &m_cells[x][y];
            resetCell(temp);
            
            temp->x = x;
            temp->y = y;
        }
    }

    createViruses(m_startViruses);
}

void Alife::createViruses(int amount){

    for(int i = m_livingCells.size(); i < amount; i++) {
        int x = qrand() % m_width;
        int y = qrand() % m_height;
        
        struct cell *temp = &m_cells[x][y];
        //cell already alive
        if(temp->alive) {
            i--;
        } else {
            temp->alive = true;
            temp->energy = 255;
            temp->code = new uchar[VIRUS_GENOME_SIZE];
            memset(temp->code, 0, VIRUS_GENOME_SIZE);

            //initial code
            for(int i = 0; i < 7; i++) {
                temp->code[i] = randomCode();
            }

            m_livingCells.append(temp);
        }
    }
}

void Alife::resetCell(struct cell *temp)
{
    temp->alive = false;
    temp->energy = 0;
    temp->code = 0;
    temp->age = 0;
    temp->killMe = false;
}



uchar Alife::randomCode()
{
    return qrand() % 19;
}

void Alife::executeCell(int id)
{
    struct cell* cell = m_livingCells.at(id);
  
    //kDebug() << cell->energy;

    if(cell->killMe)
        return;

    cell->age++;
    int facing = 0; //looking in direction
    int reg = 0;
    int codePointer = 0;

    bool stop = false;
    int max = 300;
    int special = 4;
    QRgb pixel = m_image.pixel(cell->x, cell->y); //current pixel
    QRgb pixelBackup = pixel;
  
    while(!stop && cell->energy > 0 && max) {
        cell->energy--;
        switch(cell->code[codePointer]){
            case 0: //stop
                stop = true;
            break;
            case 1://random value
                reg = qrand() % 4;
            break;
            case 2: //change direction
                facing = reg & 3;
            break;
            case 3: //move
                if(moveCell(id, facing)){
		    if(pixelBackup != pixel) {
			m_image.setPixel(cell->x, cell->y, pixel );
		    }
                    cell = m_livingCells.at(id);
                    pixel = m_image.pixel(cell->x, cell->y);
                    special--;
		    if(special <= 0) {
			stop = true;
		    }
                }
            break;
            case 4:{ //eat red
                int color = qRed(pixel);
                if(color && cell->energy + color < 255) {
                    int temp = color / 10;
                    color -= temp;
                    cell->energy += temp;
                }
                pixel = qRgb(color, qGreen(pixel), qBlue(pixel));
            }break;
            case 5:{ //eat green
                int color = qGreen(pixel);
                if(color && cell->energy + color < 255) {
                    int temp = color / 10;
                    color -= temp;
                    cell->energy += temp;
                }
                pixel = qRgb(qRed(pixel), color, qBlue(pixel));
            }break;
            case 6:{ //eat blue
                int color = qBlue(pixel);
                if(color && cell->energy + color < 255) {
                    int temp = color / 10;
                    color -= temp;
                    cell->energy += temp;
                }
                pixel = qRgb(qRed(pixel), qGreen(pixel), color);
            }break;
            case 7: //reproduce
                reproduce(cell, facing);
                special-= 2;
		if(special <= 0) {
		    stop = true;
		}
            break;
            case 8: //increment register
                reg++;
                if(reg > 255) {
                    reg = 0;
                }
            break;
            case 9: //decrement register
                reg--;
                if(reg < 0) {
                    reg = 255;
                }
            break;
            case 10: //while(register){
                if(!reg){
                    while(codePointer < VIRUS_GENOME_SIZE && cell->code[codePointer] != 11 && cell->energy){
                          codePointer++;
                          cell->energy--;
                    }
                }
            break;
            case 11: //}
                if(reg){
                    while(codePointer >= 0 && cell->code[codePointer] != 10 && cell->energy){
                          codePointer--;
                          cell->energy--;
                    }
                }
            break;
            case 12:{ //highest red
                int best = 0;
                int bestColor = 0;
                for(int i = 0; i < 4; i++){
                    QPoint neighbour = getNeighbour(cell->x, cell->y, i);
                    QRgb pixel = m_image.pixel(neighbour.x(),neighbour.y());
                    int color = qRed(pixel);
                    if(color > bestColor){
                        bestColor = color;
                        best = i;
                    }
                }
                reg = best;
            }break;
            case 13:{ //highest green
                int best = 0;
                int bestColor = 0;
                for(int i = 0; i < 4; i++){
                    QPoint neighbour = getNeighbour(cell->x, cell->y, i);
                    QRgb pixel = m_image.pixel(neighbour.x(),neighbour.y());
                    int color = qGreen(pixel);
                    if(color > bestColor){
                        bestColor = color;
                        best = i;
                    }
                }
                reg = best;
            }break;
            case 14:{ //highest blue
                int best = 0;
                int bestColor = 0;
                for(int i = 0; i < 4; i++){
                    QPoint neighbour = getNeighbour(cell->x, cell->y, i);
                    QRgb pixel = m_image.pixel(neighbour.x(),neighbour.y());
                    int color = qBlue(pixel);
                    if(color > bestColor){
                        bestColor = color;
                        best = i;
                    }
                }
                reg = best;
             }break;
            case 15:{ //empty neighbour
              int best = 0;
              for(int i = 0; i < 4; i++){
                  QPoint neighbour = getNeighbour(cell->x, cell->y, i);
                  struct cell *temp = &m_cells[neighbour.x()][neighbour.y()];
                  if(!temp->alive){
                      best = i;
                  }
              }
              reg = best;
            }break;
            case 16: // NOP
            break;
            case 17:{ //kill facing
                QPoint neighbour = getNeighbour(cell->x, cell->y, facing);
                struct cell *temp = &m_cells[neighbour.x()][neighbour.y()];
                if(temp->alive && temp->code[0] == reg) {
                    temp->killMe = true;
                    cell->energy += temp->energy / 2;
                }
            }break;
            case 18:{ //give
                QPoint neighbour = getNeighbour(cell->x, cell->y, facing);
                struct cell *temp = &m_cells[neighbour.x()][neighbour.y()];
                temp->energy += cell->energy / 2;
                cell->energy /= 2;
            }break;
            default:
              kDebug() << "wah" << cell->code[codePointer] << codePointer;
            break;
        }
        codePointer++;
        if(codePointer >= VIRUS_GENOME_SIZE) {
            stop = true;
        }
        max--;
    }

    if(pixelBackup != pixel) {
	m_image.setPixel(cell->x, cell->y, pixel );
    }

    if(cell->energy <= 0) {
        cell->energy = 0;
        cell->killMe = true;
    }
}

bool Alife::reproduce(struct cell* cell, int direction)
{
    QPoint neighbour = getNeighbour(cell->x, cell->y, direction);
    
    struct cell* newCell = &m_cells[neighbour.x()][neighbour.y()];

    if(!newCell->alive && m_livingCells.size() < m_maxViruses) {
        resetCell(newCell);
        newCell->alive = true;
        newCell->code = new uchar[VIRUS_GENOME_SIZE];
        memset(newCell->code, 0, VIRUS_GENOME_SIZE);

        newCell->energy = cell->energy / 2;
        cell->energy = cell->energy / 2;

        memcpy(newCell->code,cell->code,VIRUS_GENOME_SIZE);

        int mutate = qrand() % 2;
        if(mutate) {
            //normal mutation
            int mutations = qrand() % 6;
            for(int i = 0; i < mutations; i++) {
                int index = qrand() % VIRUS_GENOME_SIZE;
                newCell->code[index] = randomCode();
            }

            int duplication = qrand() % 4;
            for(int i = 0; i < duplication; i++) {
                int start = qrand() % VIRUS_GENOME_SIZE;
                int end = start + qrand() % ( VIRUS_GENOME_SIZE - start);
                memcpy(&newCell->code[end],&cell->code[start],VIRUS_GENOME_SIZE - end);
            }

            int deletion = qrand() % 4;
            for(int i = 0; i < deletion; i++) {
                int start = qrand() % VIRUS_GENOME_SIZE;
                int end = start + qrand() % ( VIRUS_GENOME_SIZE - start);
                memcpy(&newCell->code[start],&cell->code[end],VIRUS_GENOME_SIZE - end);
                memset(&newCell->code[end], 0, VIRUS_GENOME_SIZE - end);
            }
        }

        m_livingCells.append(newCell);
        return true;
    }
    return false;
}

bool Alife::moveCell(int i, int direction) 
{
    struct cell* cell = m_livingCells.at(i);
    QPoint neighbour = getNeighbour(cell->x, cell->y, direction);

    struct cell* newCell = &m_cells[neighbour.x()][neighbour.y()];
    if(!newCell->alive) {
        newCell->code = cell->code;
        newCell->alive = true;
        newCell->energy += cell->energy;
        newCell->age = cell->age;

        resetCell(cell);
        cell->energy = 0;
        m_livingCells[i] = newCell;
        return true;
    }

    return false;
}

//performance critical
int Alife::normalXY(int coord, int max)
{
    if(coord < 0) {
        return max;
    }

    if(coord > max) {
        return 0;
    }

    return coord;
}

QPoint Alife::getNeighbour(int x, int y, int direction) 
{
    if(direction == 0 || direction == 2) {
	y--;
        y = normalXY(y + direction, m_height - 1);
        return QPoint(x,y);
    }

    x-=2;
    x = normalXY(x + direction, m_width -1);
    return QPoint(x,y);
}

//performance critical
QImage Alife::virusMove()
{
    //kDebug() << m_max_attended << m_livingCells.size() << m_maxViruses;
    if(m_livingCells.size() < m_startViruses / 3) {
	createViruses(m_startViruses);
    }

    if(!m_max_attended && m_livingCells.size() > m_maxViruses / 2) {
	m_max_attended = true;
    }

    if(m_max_attended && m_livingCells.size() < 4 * m_startViruses) {
	m_image = m_image_original;
	m_max_attended = false;
    }
/*
    struct cell* myCell = m_livingCells.at(0);
    int pointer = 0;
    kDebug() << "start code";
    while(myCell->code[pointer] != 0 && pointer < VIRUS_GENOME_SIZE) {
        kDebug() << myCell->code[pointer++];
    }
    kDebug() << "end code";
*/
    int cells = m_livingCells.size();
    for(int i = 0; i < cells; i++) {
        executeCell(i);
    }

    //kill old cells
    for(int i = m_livingCells.size() - 1; i >= 0; i--) {
        struct cell* cell = m_livingCells.at(i);
        if(cell->age > MAX_AGE || cell->killMe) {
            if(cell->alive)
                delete [] cell->code;

            resetCell(cell);
            m_livingCells.removeAt(i);
        }
    }

    /*QImage newImage = m_image;
    for(int x = 0; x < m_width; x++) {
        for(int y = 0; y < m_height; y++) {
            struct cell *cell = &m_cells[x][y];
            newImage.setPixel(cell->x,cell->y,qRgb(cell->energy,cell->energy,cell->energy));
        }
    }
    m_pixmap = QPixmap::fromImage(newImage);*/
        
       
    
    if(m_showCells) {
        //add virus overlay
        QImage newImage = m_image;
        int size =  m_livingCells.size();
        for(int i = 0; i < size; i++) {
            struct cell* cell = m_livingCells.at(i);
            newImage.setPixel(cell->x,cell->y,qRgb(255,255,255));
        }

        return newImage;
    } else {
        return m_image;
    }
}
