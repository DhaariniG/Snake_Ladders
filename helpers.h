#ifndef HELPERS_H
#define HELPERS_H

#include "game.h"

int inBounds(int f, int x, int y);
void safeSetType(Cell maze[FLOORS][WIDTH][LENGTH], int f, int x, int y, int type);
void safeSetTarget(Cell maze[FLOORS][WIDTH][LENGTH], int f, int x, int y, int targetFloor, int targetX, int targetY, int type);
int manhattanDistance(int f1, int x1, int y1, int f2, int x2, int y2);
const char* directionToString(int dir);

int rollMovementDice();
int rollDirectionDice();

#endif
