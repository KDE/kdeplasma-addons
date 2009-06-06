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
#include <QThread>
#include <QTime>
#include <QMutex>

struct cell {
    bool alive;
    int y;
    int x;
    uchar energy;
    uchar *code;
    int age;
    bool killMe;
    uchar r;
    uchar g;
    uchar b;
};

class Alife : public QThread {
  Q_OBJECT
  public:
    Alife();
    virtual ~Alife();

    QImage currentImage(){return m_current;};
    void run();
    void virusMove();
    void initVirus();
    int normalXY(int coord, int max);
    void executeCell(int i);
    bool moveCell(int i, int direction);
    bool reproduce(struct cell *temp, int direction, QRgb prob);
    QPoint getNeighbour(int x, int y, int direction);
    uchar randomCode();
    void resetCell(struct cell *temp);
    void resetLife();
    
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

    
    void setImage(QImage image);

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

    bool m_max_attended;

    QImage m_image;
    QImage m_image_original;
    QImage m_current;
    
    int m_current_eat;
    int m_current_eat_best;
    QMutex mutex;
};

#endif
