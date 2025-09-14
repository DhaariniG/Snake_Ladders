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

int movePlayer(Player *p, int steps, Cell maze[FLOORS][WIDTH][LENGTH],
               Stairmode stairMode, Player players[], Flag flag) {
    int actualSteps = steps;

    // If poisoned
    if (p->disabledTurns >= 1) {
        return 0;
    }

    // record actual starting position (real player state)
    int startF = p->floor, startX = p->x, startY = p->y;

    // simulation state (we only commit to p at the end)
    int tempX = startX, tempY = startY, tempF = startF;
    int tempMP = p->movePoints;
    int remainingSteps = actualSteps;
    int cellsMoved = 0;
    int totalCost = 0;

    // ---------- Pre-move messages (show starting pos explicitly) ----------
    if (p->triggeredTurns > 0) {
        actualSteps *= 2;
        p->triggeredTurns--;
        printf("Player %c is triggered and rolls %d on the movement dice and moves in the %s by %d cells (starting at [%d,%d,%d]).\n",
               p->id, steps, directionToString(p->direction), actualSteps, startF, startX, startY);
    }
    else if (p->disorientedTurns > 0) {
        p->disorientedTurns--;
        printf("Player %c rolls %d on the movement dice and is disoriented (starting at [%d,%d,%d]) and will move %d cells.\n",
               p->id, steps, startF, startX, startY, steps);
    }
    else if (p->disorientedTurns == 0) {
        printf("Player %c has recovered from disorientation.\n", p->id);
        p->disorientedTurns = -1;
    }
    else if (p->turnCount % 4 == 0) {
        int newDir = rollDirectionDice();
        if (newDir != -1) {
            p->direction = newDir;
            printf("Player %c rolls %d on the movement dice and %d on the direction dice, changes direction to %s (starting at [%d,%d,%d]) and will move %d cells.\n",
                   p->id, steps, newDir, directionToString(p->direction), startF, startX, startY, steps);
        } else {
            printf("Player %c rolls %d on the movement dice (starting at [%d,%d,%d]) and will move %s by %d cells.\n",
                   p->id, steps, startF, startX, startY, directionToString(p->direction), steps);
        }
    } else {
        printf("Player %c rolls %d on the movement dice (starting at [%d,%d,%d]) and will move %s by %d cells.\n",
               p->id, steps, startF, startX, startY, directionToString(p->direction), steps);
    }

    // ---------- Simulate the movement (tempX/tempY/tempF) ----------
    while (remainingSteps > 0 && tempMP > 0) {
        int nextX = tempX, nextY = tempY, nextF = tempF;
        switch (p->direction) {
            case 0: nextX--; break; // North
            case 1: nextY++; break; // East
            case 2: nextX++; break; // South
            case 3: nextY--; break; // West
        }

        // blocked or out of bounds
        if (!inBounds(nextF, nextX, nextY) ||
            maze[nextF][nextX][nextY].type == 1 ||
            maze[nextF][nextX][nextY].type == 5) {

            tempMP -= 2; // penalty for failed move
            // show the last valid location (tempX,tempY,tempF)
            printf("Player %c cannot complete the full move due to a blocked path. Player remains at [%d,%d,%d].\n",
                   p->id, p->floor, p->x, p->y);

            if (tempMP <= 0) {
                p->movePoints = tempMP;
                printf("Player %c movement points are depleted and requires replenishment. Transporting to Bawana.\n", p->id);
                teleportToBawana(p); // note: this modifies p
                Cell *bawanaCell = &maze[p->floor][p->x][p->y];
                printf("Player %c is placed on a %d and effects take place.\n", p->id, bawanaCell->type);
                applyBawanaEffect(p, bawanaCell); // may set disabledTurns etc.
            } else {
                p->movePoints = tempMP;
            }
            return 0;
        }

        // commit this step to simulated state
        tempX = nextX; tempY = nextY; tempF = nextF;
        cellsMoved++;

        Cell *cell = &maze[tempF][tempX][tempY];
        totalCost += cell->consumable;
        tempMP -= cell->consumable;
        tempMP += cell->bonusAdd;
        if (cell->bonusMul > 0) tempMP *= cell->bonusMul;

        if (tempMP > MAX_MP) tempMP = MAX_MP;

        // prevent immediate back-and-forth by checking history
        for (int i = (p->history > 5 ? p->history - 5 : 0); i < p->history; i++) {
            if (p->lastF[i] == tempF && p->lastX[i] == tempX && p->lastY[i] == tempY) {
                return 0;
            }
        }

        // stair cell handling (logs/prints show positions based on simulated state)
        if (cell->type == 2) {
            int stairIndex = selectBestStair(cell, flag, stairMode, tempF);
            if (stairIndex >= 0) {
                int newF = cell->stairTargetFloors[stairIndex];
                int newX = cell->stairTargetXs[stairIndex];
                int newY = cell->stairTargetYs[stairIndex];
                printf("Player %c lands on [%d,%d,%d] which is a stair cell. Player %c takes the stairs and is now placed at [%d,%d,%d] in floor %d.\n",
                       p->id, tempF, tempX, tempY, p->id, newF, newX, newY, newF);
                tempF = newF; tempX = newX; tempY = newY;
            } else {
                return 0;
            }
        }
        else if (cell->type == 3) {
            int newF = cell->targetFloor;
            int newX = cell->targetX;
            int newY = cell->targetY;
            printf("Player %c lands on [%d,%d,%d] which is a pole cell. Player %c slides down and is now placed at [%d,%d,%d] in floor %d.\n",
                   p->id, tempF, tempX, tempY, p->id, newF, newX, newY, newF);
            tempF = newF; tempX = newX; tempY = newY;
        }

        remainingSteps--;
    }

    // ran out of MP during simulation
    if (tempMP <= 0) {
        printf("Player %c movement points are depleted and requires replenishment. Transporting to Bawana.\n", p->id);
        teleportToBawana(p); // modifies p
        Cell *bawanaCell = &maze[p->floor][p->x][p->y];
        printf("Player %c is placed on a %d and effects take place.\n", p->id, bawanaCell->type);
        applyBawanaEffect(p, bawanaCell);
        return 0;
    }

    // commit simulated state to real player
    p->x = tempX; p->y = tempY; p->floor = tempF;
    p->movePoints = tempMP;

    addHistory(p);
    checkCapture(p, players);

    if (checkWin(p, flag)) return 2;

    printf("Player %c moved %d cells that cost %d movement points and is now at [%d,%d,%d].\n",
           p->id, cellsMoved, totalCost, tempF, tempX, tempY);

    return 0;
}
