#ifndef REACHABILITY_H
#define REACHABILITY_H


#include <stdbool.h>
#include <string.h>
#include "game.h"

typedef struct {
    int f, x, y;
} CellPosition;

int checkReachability(Cell maze[FLOORS][WIDTH][LENGTH], int startF, int startX, int startY, Flag flag);


#endif