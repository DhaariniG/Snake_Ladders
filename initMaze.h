#ifndef INITMAZE_H
#define INITMAZE_H

#include "game.h"

void initMaze(Cell maze[FLOORS][WIDTH][LENGTH], Flag *flag, Stairmode stairMode);
void addStandingArea(Cell maze[FLOORS][WIDTH][LENGTH]);
void addVoidSpace(Cell maze[FLOORS][WIDTH][LENGTH]);
void addWalls(Cell maze[FLOORS][WIDTH][LENGTH], const char *filename);
void addFlag(Cell maze[FLOORS][WIDTH][LENGTH], Flag *flag, const char *filename);
void addStairs(Cell maze[FLOORS][WIDTH][LENGTH], const char *filename);
int addStairTarget(Cell maze[FLOORS][WIDTH][LENGTH], int f, int x, int y, int targetFloor, int targetX, int targetY);
int selectBestStair(Cell *cell, Flag flag, Stairmode stairMode, int currentFloor);
void addPoles(Cell maze[FLOORS][WIDTH][LENGTH], const char *filename);
void addConsumables(Cell maze[FLOORS][WIDTH][LENGTH]);
void addBawana(Cell maze[FLOORS][WIDTH][LENGTH]);
Stairmode currentMode(int roundcount);
int addSEED(const char *filename);

#endif
