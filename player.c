#include <stdio.h>   
#include <stdlib.h>  
#include "game.h"    
#include "helpers.h"   
#include "initMaze.h"   
#include "bawana.h" 

void addHistory(Player *p) {
    if (p->history >= 10) {
        for (int i = 0; i < 9; i++) {
            p->lastF[i] = p->lastF[i+1];
            p->lastX[i] = p->lastX[i+1];
            p->lastY[i] = p->lastY[i+1];
        }
        p->history = 9;
    }

    p->lastF[p->history] = p->floor;
    p->lastX[p->history] = p->x;
    p->lastY[p->history] = p->y;
    p->history++;
}

int checkWin(Player *p, Flag flag) {
    if (p->floor == flag.floor && p->x == flag.x && p->y == flag.y) {
        printf("=======================================\n");
        printf("Player %c has captured the flag!\n", p->id);
        printf("=======================================\n");

        fprintf(logFile, "Player %c captured the flag at [%d,%d,%d]\n",
                p->id, p->floor, p->x, p->y);
        return 1;
    }
    return 0;
}

void checkCapture(Player *currentPlayer, Player players[]) {
    if (isInBawana(currentPlayer->floor, currentPlayer->x, currentPlayer->y)) {
        return;
    }

    for (int i = 0; i < PLAYERS; i++) {
        Player *otherPlayer = &players[i];
        if (otherPlayer->id == currentPlayer->id) {
            continue; 
        }

        if (otherPlayer->inMaze &&
            otherPlayer->x == currentPlayer->x &&
            otherPlayer->y == currentPlayer->y &&
            otherPlayer->floor == currentPlayer->floor) {

            printf("Player %c captures Player %c! Player %c returns to the starting area.\n", currentPlayer->id, otherPlayer->id, otherPlayer->id);
            otherPlayer->x = otherPlayer->startX;
            otherPlayer->y = otherPlayer->startY;
            otherPlayer->floor = otherPlayer->startFloor;
            otherPlayer->inMaze = 0;
        }
    }
}

int movePlayer(Player *p, int steps, Cell maze[FLOORS][WIDTH][LENGTH], Stairmode stairMode, Player players[], Flag flag) {
    int actualSteps = steps;
    if (p->triggeredTurns > 0) actualSteps *= 2; 

    int tempX = p->x, tempY = p->y, tempF = p->floor;
    int tempMP = p->movePoints;
    int remainingSteps = actualSteps;
    int cellsMoved = 0;
    int totalCost = 0;

    while (remainingSteps > 0 && tempMP > 0) {
        int nextX = tempX, nextY = tempY, nextFloor = tempF;

        switch (p->direction) {
            case 0: nextX--; break; // North
            case 1: nextY++; break; // East
            case 2: nextX++; break; // South
            case 3: nextY--; break; // West
        }

        if (!inBounds(nextFloor, nextX, nextY) || maze[nextFloor][nextX][nextY].type == 1 || maze[nextFloor][nextX][nextY].type == 5) {
            
            tempMP -= 2;

            printf("%c rolls and %d on the movement dice and cannot move in the %s direction. Player remains at [%d,%d,%d]\n", p->id, steps, directionToString(p->direction), p->floor, p->x, p->y);
            
           
            if (tempMP <= 0) {
                p->movePoints = tempMP;
                printf("Player %c movement points are depleted and requires replenishment. Transporting to Bawana.\n", p->id);
                teleportToBawana(p);
                Cell *bawanaCell = &maze[p->floor][p->x][p->y];
                printf("Player %c is place on a %d and effects take place.\n", p->id, bawanaCell->type);
                applyBawanaEffect(p, bawanaCell);
            } else {
                p->movePoints = tempMP;
            }
            return 0;
        }

        tempX = nextX; tempY = nextY; tempF = nextFloor;
        cellsMoved++;
        
        Cell *cell = &maze[tempF][tempX][tempY];
        
        totalCost += cell->consumable;
        tempMP -= cell->consumable;
        tempMP += cell->bonusAdd;
        if (cell->bonusMul > 0) tempMP *= cell->bonusMul;

        
        if (tempMP > MAX_MP) {
            tempMP = MAX_MP;
        }

        for (int i = (p->history > 5 ? p->history - 5 : 0); i < p->history; i++) {
        if (p->lastF[i] == tempF && p->lastX[i] == tempX && p->lastY[i] == tempY) {
            printf("Player %c detected in a movement loop! Transporting to starting area.\n", p->id);
            
            p->floor = p->startFloor;
            p->x = p->startX;
            p->y = p->startY;
            p->inMaze = 0; 
        
            p->movePoints = tempMP;
            
            return 0; 
        }
    }

        if (cell->type == 2) {
            int stairIndex = selectBestStair(cell, flag, stairMode, tempF);
            if (stairIndex >= 0) {
                int newF = cell->stairTargetFloors[stairIndex];
                int newX = cell->stairTargetXs[stairIndex];
                int newY = cell->stairTargetYs[stairIndex];
                printf("Player %c lands on [%d,%d,%d] which is a stair cell. Player %c takes the stairs and now placed at [%d,%d,%d] in floor %d.\n",
                       p->id, tempF, tempX, tempY, p->id, newF, newX, newY, newF);
                tempF = newF; tempX = newX; tempY = newY;
            } else {
                return 0;
            }
        } else if (cell->type == 3) {
            int newF = cell->targetFloor;
            int newX = cell->targetX;
            int newY = cell->targetY;
            printf("Player %c lands on [%d,%d,%d] which is a pole cell. Player %c slides down and now placed at [%d,%d,%d] in floor %d.\n",
                   p->id, tempF, tempX, tempY, p->id, newF, newX, newY, newF);
            tempF = newF; tempX = newX; tempY = newY;
        }

        remainingSteps--;
    }

    if (tempMP <= 0) {
        printf("Player %c movement points are depleted and requires replenishment. Transporting to Bawana.\n", p->id);
        teleportToBawana(p);
        Cell *bawanaCell = &maze[p->floor][p->x][p->y];
        printf("Player %c is place on a %d and effects take place.\n", p->id, bawanaCell->type);
        applyBawanaEffect(p, bawanaCell);
        return 0;
    }

    p->x = tempX; p->y = tempY; p->floor = tempF;
    p->movePoints = tempMP;

    addHistory(p);
    checkCapture(p, players);

    if (checkWin(p, flag)) return 2;

    printf("%c moved %d cells that cost %d movement points and is left with %d and is moving in the %s.\n",
           p->id, actualSteps, totalCost, p->movePoints, directionToString(p->direction));

    return 0;
}


