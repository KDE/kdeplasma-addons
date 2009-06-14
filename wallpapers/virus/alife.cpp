/*
  Copyright (c) 2009 by Beat Wolf <asraniel@fryx.ch>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.
*/

#include "alife.h" 

#include <KDebug>

#define VIRUS_GENOME_SIZE 38
#define MAX_AGE 8

#define MAX_EAT 10
#define MIN_EAT 1

Alife::Alife(){
    m_cells = 0;
    m_height = 0;
    m_width = 0;
    m_max_attended = false;
    m_current_eat = MAX_EAT;
    m_current_eat_best = MIN_EAT;
}

Alife::~Alife(){
    resetLife();
}

void Alife::resetLife(){
    mutex.lock();
    if(inited()){
        while( !m_livingCells.isEmpty() ){
            struct cell* cell = m_livingCells.takeFirst();
            delete [] cell->code;
        }

        delete []m_cells[0];
        delete []m_cells;
        m_cells = 0;
    }
    mutex.unlock();
    //qDebug() << "life reseted";
}

void Alife::setImage(QImage image){
    bool keepViruses = false;
    m_image = image;
    m_image_original = image;
    keepViruses = m_height == m_image.height() && m_width == m_image.width();
    m_height = m_image.height();
    m_width = m_image.width();
    m_max_attended = false;
    if(!keepViruses || !inited()){
        resetLife();
        initVirus();
    }
}

void Alife::initVirus(){
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
    //qDebug() << "life created";
}

//create the needed viruses so that we have in total "amount" viruses
void Alife::createViruses(int amount){
    for(int i = m_livingCells.size(); i < amount; i++) {
        int x = rand() % m_width;
        int y = rand() % m_height;
        
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
                temp->code[i] = rand()%12;
            }

	    temp->code[rand()%7] = 7; //cheating, jumpstart evolution

	    /*temp->code[0] = 4;
	    temp->code[1] = 12;
	    temp->code[2] = 2;
	    temp->code[3] = 7;*/

	    /*temp->code[0] = 1;
	    temp->code[1] = 10;
	    temp->code[2] = 19;
	    temp->code[3] = 4;
	    temp->code[4] = 19;
	    temp->code[5] = 6;
	    temp->code[6] = 19;
	    temp->code[7] = 6;
	    temp->code[8] = 12;
	    temp->code[9] = 2;
	    temp->code[10] = 7;
	    temp->code[11] = 13;

	    temp->code[12] = 2;
	    temp->code[13] = 3;
	    temp->code[14] = 9;
	    temp->code[15] = 11;
	    temp->code[16] = 7;*/
            m_livingCells.append(temp);
        }
    }
}

//resets the values of a cell
void Alife::resetCell(struct cell *temp)
{
    temp->alive = false;
    temp->energy = 0;
    temp->code = 0;
    temp->age = 0;
    temp->killMe = false;
    temp->r = 0;
    temp->g = 0;
    temp->b = 0;
}

//returns a random operation for the genome. 
//REMINDER: Update if a new op is added
uchar Alife::randomCode()
{
    return rand() % 20;
}

//Executes the code of a certain living cell
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

    int better_eat = m_current_eat; //what inverse percentage is eaten next time

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
                reg = rand() % 4;
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
                    pixelBackup = pixel;
                    special--;
		    if(special <= 0) {
			stop = true;
		    }
                }
            break;
            case 4:{ //eat red
                int color = qRed(pixel);
                if(color && cell->energy + color < 255) {
                    int temp = qMin(color / better_eat, color);
                    color -= temp;
                    cell->energy += temp;
		    better_eat = m_current_eat;
                }
                pixel = qRgb(color, qGreen(pixel), qBlue(pixel));
            }break;
            case 5:{ //eat green
                int color = qGreen(pixel);
                if(color && cell->energy + color < 255) {
                    int temp = qMin(color / better_eat, color);
                    color -= temp;
                    cell->energy += temp;
		    better_eat = m_current_eat;
                }
                pixel = qRgb(qRed(pixel), color, qBlue(pixel));
            }break;
            case 6:{ //eat blue
                int color = qBlue(pixel);
                if(color && cell->energy + color < 255) {
                    int temp = qMin(color / better_eat, color);
                    color -= temp;
                    cell->energy += temp;
		    better_eat = m_current_eat;
                }
                pixel = qRgb(qRed(pixel), qGreen(pixel), color);
            }break;
            case 7: //reproduce
                reproduce(cell, facing, pixel);
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
                    while(codePointer < VIRUS_GENOME_SIZE-1 && cell->code[codePointer] != 11 && cell->energy){
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
		    int enTemp = cell->energy + temp->energy / 2;
                    cell->energy = qMin(255, enTemp);
                }
            }break;
            case 18:{ //give
                QPoint neighbour = getNeighbour(cell->x, cell->y, facing);
                struct cell *temp = &m_cells[neighbour.x()][neighbour.y()];
		int enTemp = temp->energy + cell->energy / 2;
                temp->energy = qMin(255, enTemp);
                cell->energy /= 2;
            }break;
	    case 19:{ //better eat
		better_eat = m_current_eat_best;
	    }break;
            default:
              kDebug() << "wah" << cell->code[codePointer] << codePointer;
            break;
        }
        codePointer++;
        if(codePointer >= VIRUS_GENOME_SIZE) {
            stop = true;
        }
	better_eat = qMin(better_eat + 1, m_current_eat);
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

bool Alife::reproduce(struct cell* cell, int direction, QRgb color)
{
    QPoint neighbour = getNeighbour(cell->x, cell->y, direction);
    
    struct cell* newCell = &m_cells[neighbour.x()][neighbour.y()];

    if(!newCell->alive&& m_livingCells.size() < m_maxViruses) {

	//give a unfair advantage to darker places
	int prob = (((qRed(color) + qGreen(color) + qBlue(color))/ 255.)+1);// 0 - 765

	if(rand() % prob  != 0){
	    return false;
	}

	resetCell(newCell);
        newCell->alive = true;
        newCell->code = new uchar[VIRUS_GENOME_SIZE];
        memset(newCell->code, 0, VIRUS_GENOME_SIZE);

        newCell->energy = cell->energy / 3;
        cell->energy = cell->energy / 3;

        memcpy(newCell->code,cell->code,VIRUS_GENOME_SIZE);

        int mutate = rand() % 3;
        if(mutate) {
            //normal mutation
            int mutations = rand() % 5;
            for(int i = 0; i < mutations; i++) {
                int index = rand() % VIRUS_GENOME_SIZE;
                newCell->code[index] = randomCode();
            }

            int duplication = rand() % 3;
            for(int i = 0; i < duplication; i++) {
                int start = rand() % VIRUS_GENOME_SIZE;
                int end = start + rand() % ( VIRUS_GENOME_SIZE - start);
                memcpy(&newCell->code[end],&cell->code[start],VIRUS_GENOME_SIZE - end);
            }

            int deletion = rand() % 3;
            for(int i = 0; i < deletion; i++) {
                int start = rand() % VIRUS_GENOME_SIZE;
                int end = start + rand() % ( VIRUS_GENOME_SIZE - start);
                memcpy(&newCell->code[start],&cell->code[end],VIRUS_GENOME_SIZE - end);
                memset(&newCell->code[end], 0, VIRUS_GENOME_SIZE - end);
            }
        }
  
	int r = 0;
	int g = 0;
	int b = 0;
	int mod = 1;
	for(int i = 0; i < VIRUS_GENOME_SIZE; i++) {
	    switch(newCell->code[i]) {
		case 4:
		  r += 40 * mod;
		break;
		case 5:
		  g += 40 * mod;
		break;
		case 6:
		  b += 40 * mod;
		break;
		case 10:
		  mod = 2;
		break;
		case 11:
		  mod = 1;
		break;
		case 12:
		  r += 20 * mod;
		break;
		case 13:
		  g += 20 * mod;
		break;
		case 14:
		  b += 20 * mod;
		break;
	    }
	}

	newCell->r = qMin(r, 255);
	newCell->g = qMin(g, 255);
	newCell->b = qMin(b, 255);

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
        newCell->energy += cell->energy / 2;
        newCell->age = cell->age;
	newCell->r = cell->r;
	newCell->g = cell->g;
	newCell->b = cell->b;

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

void Alife::run(){
    mutex.lock();
    QTime a = QTime::currentTime();
    qsrand(a.msec());
    
    virusMove();
    
    QTime b = QTime::currentTime();
    //qDebug() << "needed" << a.msecsTo(b);

    mutex.unlock();
}

//performance critical
void Alife::virusMove()
{
    m_current_eat = qMax(MAX_EAT, (int)(((double)m_livingCells.size()/ (double) (m_maxViruses / 4.0)) * MAX_EAT));
    m_current_eat_best = qMax(MIN_EAT, (int)(((double)m_livingCells.size()/ (double) (m_maxViruses / 4.0)) * (MIN_EAT * 2)));
    //kDebug() << m_current_eat_best << m_current_eat << m_livingCells.size() << m_maxViruses;
    if(m_livingCells.size() < m_startViruses / 3) {
	createViruses(m_startViruses);
    }

    if(!m_max_attended && m_livingCells.size() > m_maxViruses / 10) {
	m_max_attended = true;
    }

    if(m_max_attended && m_livingCells.size() < 4 * m_startViruses) {
	m_image = m_image_original;
	m_max_attended = false;
    }

    /*struct cell* myCell = m_livingCells.at(0);
    int pointer = 0;
    kDebug() << "start code";
    while(myCell->code[pointer] != 0 && pointer < VIRUS_GENOME_SIZE) {
        kDebug() << myCell->code[pointer++];
    }
    kDebug() << "end code";*/

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
            newImage.setPixel(cell->x,cell->y,qRgb(cell->r,cell->g,cell->b));
        }

        m_current = newImage;
    } else {
        m_current = m_image;
    }
}
