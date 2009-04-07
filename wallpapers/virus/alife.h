/*
  Copyright (c) 2009 by Beat Wolf <asraniel@fryx.ch>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.
*/

#ifndef ALIFE_HEADER
#define ALIFE_HEADER

#include <QObject>
#include <QImage>
#include <QPoint>

struct cell {
    bool alive;
    int y;
    int x;
    uchar energy;
    uchar *code;
    int age;
    bool killMe;
};

class Alife : public QObject {
  Q_OBJECT
  public:
    Alife();
    virtual ~Alife();

    QImage virusMove();
    void initVirus();
    int normalXY(int coord, int max);
    void executeCell(int i);
    bool moveCell(int i, int direction);
    bool reproduce(struct cell *temp, int direction);
    QPoint getNeighbour(int x, int y, int direction);
    uchar randomCode();
    void resetCell(struct cell *temp);

    bool inited(){ return m_cells != 0;};

    void setUpdateInterval(int interval){
       m_updateInterval = interval;
    };
    
    int getUpdateInterval(){ 
        return m_updateInterval;
    };

    void setStartViruses(int start){
        m_startViruses = start;
    };

    int getStartViruses(){
        return m_startViruses;
    };

    void setMaxViruses(int max){
	m_maxViruses = max;
    };

    int getMaxViruses(){
	return m_maxViruses;
    };

    void setShowCells(bool show){
	m_showCells = show;
    };

    bool showCells(){
	return m_showCells;
    }

    void setImage(QImage image){
      m_image = image;
      m_height = m_image.height();
      m_width = m_image.width();
    }

    void createViruses(int amount);

  protected:
    struct cell ** m_cells;
    QList<struct cell*>m_livingCells;

    int m_updateInterval;
    int m_startViruses;
    int m_maxViruses;
    bool m_showCells;
  
    int m_height;
    int m_width;

    QImage m_image;
};

#endif