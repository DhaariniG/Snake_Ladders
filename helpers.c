#include <stdio.h>
#include <stdlib.h>  
#include "game.h" 
#include "helpers.h"


int inBounds(int f, int x, int y) {
    return f >= 0 && f < FLOORS && x >= 0 && x < WIDTH && y >= 0 && y < LENGTH;
}

void safeSetType(Cell maze[FLOORS][WIDTH][LENGTH], int f, int x, int y, int type) {
    if (!inBounds(f, x, y)) {
        return;
    }
    maze[f][x][y].type = type;
}

void safeSetTarget(Cell maze[FLOORS][WIDTH][LENGTH], int f, int x, int y, int targetFloor, int targetX, int targetY, int type) {
    if (!inBounds(f, x, y) || !inBounds(targetFloor, targetX, targetY)) {
        return;
    }
    maze[f][x][y].type = type;
    maze[f][x][y].targetFloor = targetFloor;
    maze[f][x][y].targetX = targetX;
    maze[f][x][y].targetY = targetY;
}

int manhattanDistance(int f1, int x1, int y1, int f2, int x2, int y2) {
    return abs(f1 - f2) + abs(x1 - x2) + abs(y1 - y2);
}

const char* directionToString(int dir) {
    switch (dir) {
        case 0: return "North";
        case 1: return "East";
        case 2: return "South";
        case 3: return "West";
        default: return "Invalid";
    }
}

// Dice and Mode
int rollMovementDice() {
    return rand() % 6 + 1;
}

int rollDirectionDice() {
    int face = rand() % 6 + 1;
    switch (face) {
        case 2: return 0; // North
        case 3: return 1; // East
        case 4: return 2; // South
        case 5: return 3; // West
        default: return -1; // other sides
    }
}
