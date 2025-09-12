#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <limits.h>
#include "game.h"

FILE *logFile = NULL;  

// Helper Functions
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

Stairmode currentMode(int roundcount) {
    static Stairmode mode = stairs_bi;
    if (roundcount % 5 == 1) {
        mode = (Stairmode)(rand() % 3);
    }
    return mode;
}

// Player history
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

// Capture and Win
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

// Maze Setup
void addStandingArea(Cell maze[FLOORS][WIDTH][LENGTH]) {
    for (int i = WIDTH - 4; i < WIDTH; i++) {
        for (int j = 7; j < 16; j++) {
            maze[0][i][j].type = 1; // Walls
        }
    }
}

void addVoidSpace(Cell maze[FLOORS][WIDTH][LENGTH]) {
    for (int i = 0; i < WIDTH - 4; i++) { // Floor 2
        for (int j = 8; j < LENGTH - 8; j++) {
            maze[1][i][j].type = 5;
        }
    }

    for (int i = 0; i < WIDTH; i++) { // Floor 3
        for (int j = 0; j < 8; j++) {
            maze[2][i][j].type = 5;
        }
        for (int j = 8; j < 17; j++) {
            maze[2][i][j].type = 5;
        }
    }

    // Bawana void space
    for (int i = 6; i <= 9; i++) {
        maze[0][i][20].type = 5;
    }
    for (int j = 20; j <= 24; j++) {
        maze[0][6][j].type = 5;
    }
}

int addSEED(const char *filename){
    int seed = 1; // default
    FILE *fp = fopen(filename, "r");
    if (fp) {
        if (fscanf(fp, "%d", &seed) != 1) {
            fprintf(stderr, "File is malformed. Setting seed to 1\n");
        }
        fclose(fp);
    } else {
        fprintf(stderr, "Cannot open the file %s. Setting seed to 1\n", filename);
    }
    return seed;

}


void addWalls(Cell maze[FLOORS][WIDTH][LENGTH], const char *filename) {
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        fprintf(stderr, "Error opening the file walls.txt\n");
        exit(1);
    }
    int f, sx, sy, ex, ey;
    while (fscanf(fp, "[%d,%d,%d,%d,%d]\n", &f, &sx, &sy, &ex, &ey) == 5) {
        if (!inBounds(f, sx, sy) || !inBounds(f, ex, ey)) {
            fprintf(stderr, "Wall entry out of bounds: [%d,%d,%d,%d,%d] - Disregarded\n", f, sx, sy, ex, ey);
            continue;
        }
        if (sx > ex) { int t = sx; sx = ex; ex = t; }
        if (sy > ey) { int t = sy; sy = ey; ey = t; }
        if (sx == ex) {
            for (int y = sy; y <= ey; y++) {
                if (maze[f][sx][y].type == 0) 
                {safeSetType(maze, f, sx, y, 1);
                    if(y==ey) fprintf(logFile, "Wall placed from [%d,%d,%d] to [%d,%d,%d]\n", f, sx, sy, ex, ey);
                }
                else fprintf(stderr, "Cannot place wall at (%d,%d,%d) - cell already occupied by %d\n", f, sx, y, maze[f][sx][y].type);
            }
        } else if (sy == ey) {
            for (int x = sx; x <= ex; x++) {
                if (maze[f][x][sy].type == 0) {
                    safeSetType(maze, f, x, sy, 1);
                    if(x==ex) fprintf(logFile, "Wall placed from [%d,%d,%d] to [%d,%d,%d]\n", f, sx, sy, ex, ey);
                }
                else fprintf(stderr, "Cannot place wall at (%d,%d,%d) - cell already occupied by %d\n", f, x, sy,maze[f][x][sy].type);
            }
        } else {
            fprintf(stderr, "Wall entry not straight line: [%d,%d,%d,%d,%d] - Disregarded\n", f, sx, sy, ex, ey);
        }
    }
    fclose(fp);
}

void addFlag(Cell maze[FLOORS][WIDTH][LENGTH], Flag *flag, const char *filename) {
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        fprintf(stderr, "Cannot open the file %s. Exiting.\n", filename);
        exit(1);
    }

    int f, x, y;
    if (fscanf(fp, "[%d,%d,%d]\n", &f, &x, &y) == 3) {
        if (isInBawana(f, x, y)) {
            fprintf(stderr, "Flag [%d,%d,%d] lies in Bawana. Exiting.\n", f, x, y);
            fclose(fp);
            exit(1);
        } else if (maze[f][x][y].type == 1 || maze[f][x][y].type == 5) {
            fprintf(stderr, "Flag [%d,%d,%d] is inside wall/void. Exiting.\n", f, x, y);
            fclose(fp);
            exit(1);
        } else {
            flag->floor = f;
            flag->x = x;
            flag->y = y;
            safeSetType(maze, f, x, y, 4);
            fprintf(logFile, "Flag placed at [%d,%d,%d]\n", f, x, y);
        }
    } else {
        fprintf(stderr, "flag.txt malformed. Exiting.\n");
        fclose(fp);
        exit(1);
    }

    fclose(fp);
}

void addStairs(Cell maze[FLOORS][WIDTH][LENGTH], const char *filename) {
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        fprintf(stderr, "Error opening the file stairs.txt\n");
        exit(1);
    }
    int f1, x1, y1, f2, x2, y2;
    while (fscanf(fp, "[%d,%d,%d,%d,%d,%d]\n", &f1, &x1, &y1, &f2, &x2, &y2) == 6) {
        if (inBounds(f1, x1, y1) && inBounds(f2, x2, y2)) {
            addStairTarget(maze, f1, x1, y1, f2, x2, y2);
            addStairTarget(maze, f2, x2, y2, f1, x1, y1);
        }
    }
    fclose(fp);
}

int addStairTarget(Cell maze[FLOORS][WIDTH][LENGTH], int f, int x, int y, int targetFloor, int targetX, int targetY) {
    if (!inBounds(f, x, y) || !inBounds(targetFloor, targetX, targetY)) {
        return 0;
    }
    Cell *cell = &maze[f][x][y];
    if (cell->type == 0 || cell->type == 2) {
        if (cell->numStairs < 2) {
            cell->stairTargetFloors[cell->numStairs] = targetFloor;
            cell->stairTargetXs[cell->numStairs] = targetX;
            cell->stairTargetYs[cell->numStairs] = targetY;
            cell->numStairs++;
            cell->type = 2;
            fprintf(logFile, "Stair placed from [%d,%d,%d] to [%d,%d,%d]\n", f, x, y, targetFloor, targetX, targetY);

            return 1;
        } else {
            fprintf(stderr, "Cell [%d,%d,%d] already has maximum stairs (%d) - cannot add more\n",
                    f, x, y, 2);
            return 0;
        }
    } else {
        fprintf(stderr, "Cannot place stair at [%d,%d,%d] - cell occupied by type %d\n", f, x, y, cell->type);
        return 0;
    }
}

int selectBestStair(Cell *cell, Flag flag, Stairmode stairMode, int currentFloor) {
    if (cell->numStairs == 0) {
        return -1;
    }

    int bestStair = -1;
    int bestDistance = INT_MAX;

    for (int i = 0; i < cell->numStairs; i++) {
        int targetFloor = cell->stairTargetFloors[i];

        if (stairMode == stairs_bi ||
            (stairMode == stairs_up && targetFloor > currentFloor) ||
            (stairMode == stairs_down && targetFloor < currentFloor)) {

            int distance = manhattanDistance(
                targetFloor, cell->stairTargetXs[i], cell->stairTargetYs[i],
                flag.floor, flag.x, flag.y
            );

            if (distance < bestDistance) {
                bestDistance = distance;
                bestStair = i;
            } else if (distance == bestDistance) {
                 if (rand() % 2 == 0) { 
                     bestStair = i;
                 }
            }
            fprintf(logFile, "Stair leading to [%d,%d,%d] is chosen.\n",
                cell->stairTargetFloors[i],
                cell->stairTargetXs[i],
                cell->stairTargetYs[i]);
        }
    }

    return bestStair;
}

void addPoles(Cell maze[FLOORS][WIDTH][LENGTH], const char *filename) {
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        fprintf(stderr, "Error opening the file poles.txt\n");
        exit(1);
    }

    int lf, hf, x, y;
    while (fscanf(fp, "[%d,%d,%d,%d]\n", &lf, &hf, &x, &y) == 4) {

        if (lf >= hf) {
            fprintf(stderr, "Warning: pole [%d,%d,%d,%d] must have lf < hf - Disregarded\n", lf, hf, x, y);
            continue;
        }

        if (isInBawana(lf, x, y)) {
            fprintf(stderr, "Warning: pole [%d,%d,%d,%d] cannot be in bawana - Disregarded\n", lf, hf, x, y);
            continue;
        }

        for (int f = hf; f >= lf; f--) {
            if (maze[f][x][y].type != 0) {
                fprintf(stderr, "Cannot place pole at [%d,%d,%d] - cell already occupied by %d\n", f, x, y,maze[f][x][y].type);
                continue;
            }
            safeSetTarget(maze, f, x, y, lf, x, y, 3);
            if(f == lf) fprintf(logFile, "Pole placed from [%d,%d,%d] to [%d,%d,%d]\n", hf,x,y,lf,x,y);

        }
    }
    fclose(fp);
}

void initMaze(Cell maze[FLOORS][WIDTH][LENGTH], Flag *flag, Stairmode stairMode) {
    // initialize empty maze...
    for (int f = 0; f < FLOORS; f++) {
        for (int w = 0; w < WIDTH; w++) {
            for (int l = 0; l < LENGTH; l++) {
                maze[f][w][l].type = 0;
                maze[f][w][l].targetFloor = -1;
                maze[f][w][l].targetX = -1;
                maze[f][w][l].targetY = -1;
                maze[f][w][l].numStairs = 0;
                maze[f][w][l].consumable = 0;
                maze[f][w][l].bonusAdd = 0;
                maze[f][w][l].bonusMul = 0;
            }
        }
    }

    addStandingArea(maze);
    addVoidSpace(maze);
    addBawana(maze);
    addWalls(maze, "walls.txt");      
    addPoles(maze, "poles.txt");
    addStairs(maze, "stairs.txt");
    addConsumables(maze);
    
    addFlag(maze, flag, "flag.txt");
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

// BAWANA and Consumables
int isInBawana(int f, int x, int y) {
    return f == 0 && x > 6 && x <= 9 && y > 20 && y <= 24;
}

void addConsumables(Cell maze[FLOORS][WIDTH][LENGTH]) {
    for (int f = 0; f < FLOORS; f++) {
        for (int x = 0; x < WIDTH; x++) {
            for (int y = 0; y < LENGTH; y++) {
                Cell *cell = &maze[f][x][y];
                if (cell->type != 0 || isInBawana(f, x, y)) {
                    cell->consumable = 0; cell->bonusAdd = 0; cell->bonusMul = 0;
                    continue;
                }
                int r = rand() % 100;
                if (r < 25) { cell->consumable = 0; }
                else if (r < 60) { cell->consumable = (rand() % 4) + 1; }
                else if (r < 85) { cell->bonusAdd = (rand() % 2) + 1; }
                else if (r < 95) { cell->bonusAdd = (rand() % 3) + 3; }
                else { cell->bonusMul = (rand() % 2) ? 3 : 2; }
            }
        }
    }
}

void addBawana(Cell maze[FLOORS][WIDTH][LENGTH]) {
    int count = 0;
    for (int x = 7; x <= 9; x++) {
        for (int y = 21; y <= 24; y++) {
            Cell *c = &maze[0][x][y];
            if (c->type == 5) continue;
            switch (count) {
                case 0: c->type = 7; break;
                case 1: c->type = 6; break;
                case 2: c->type = 8; break;
                case 3: c->type = 7; break;
                case 6: c->type = 9; break;
                case 7: c->type = 8; break;
                case 9: c->type = 6; break;
                case 11: c->type = 9; break;
                default: {
                    c->type = 10;
                    c->bawanaValue = rand() % 91 + 10;
                    break;
                }
            }
            count++;
        }
    }
}

void teleportToBawana(Player *p) {
    int count = 0;
    int bawanaCellsX[12], bawanaCellsY[12];
    for (int x = 7; x <= 9; x++) {
        for (int y = 21; y <= 24; y++) {
            if (!inBounds(0, x, y)) continue;
            bawanaCellsX[count] = x;
            bawanaCellsY[count] = y;
            count++;
        }
    }
    int choice = rand() % count;
    p->floor = 0;
    p->x = bawanaCellsX[choice];
    p->y = bawanaCellsY[choice];
    p->inMaze = 0;
}

void applyBawanaEffect(Player *p, Cell *cell) {
    switch (cell->type) {
        case 6:
            printf("Player %c eats from Bawana and have a bad case of food poisoning. Will need three rounds to recover.\n", p->id);
            p->disabledTurns = 3;
            break;
        case 7:
            printf("Player %c eats from Bawana and is disoriented and is placed at the entrance of Bawana with 50 movement points.\n", p->id);
            p->movePoints += 50;
            break;
        case 8:
            printf("Player %c eats from Bawana and is triggered due to bad quality of food. Player %c is placed at the entrance of Bawana with 50 movement points.\n", p->id, p->id);
            p->movePoints += 50;
            break;
        case 9:
            printf("Player %c eats from Bawana and is happy. Player %c is placed at the entrance of Bawana with 200 movement points.\n", p->id, p->id);
            p->movePoints += 200;
            break;
        case 10:
            printf("Player %c eats from Bawana and earns %d movement points and is placed at the [%d,%d,%d].\n", p->id, cell->bawanaValue, p->floor, p->x, p->y);
            p->movePoints += cell->bawanaValue;
            break;
    }

    
    if (p->movePoints > MAX_MP) {
        p->movePoints = MAX_MP;
    }

   
    if (cell->type >= 7 && cell->type <= 10) {
        p->x = 9;
        p->y = 19;
        p->direction = 0;
        p->inMaze = 1;
        if (cell->type == 7) p->disorientedTurns = 4;
        if (cell->type == 8) p->triggeredTurns = 4;
    }
}

void handleBawanaEffects(Player *p, Cell maze[FLOORS][WIDTH][LENGTH]) {
    if (p->disabledTurns > 0) {
        printf("Player %c is still food poisoned and misses the turn.\n", p->id);
        p->disabledTurns--;
        if (p->disabledTurns == 0) {
            printf("Player %c is now fit to proceed from the food poisoning episode and now placed on a %d and the effects take place.\n", p->id, maze[p->floor][p->x][p->y].type);
            teleportToBawana(p);
            applyBawanaEffect(p, &maze[p->floor][p->x][p->y]);
        }
        return;
    }

    if (p->disorientedTurns > 0) {
        p->direction = rand() % 4;
        p->disorientedTurns--;
    }

    if (p->triggeredTurns > 0) {
        p->triggeredTurns--;
    }
}

void play(void) {
    int seed = addSEED("seed.txt");
    srand(seed);

    logFile = fopen("log.txt", "w");
    if (!logFile) {
        fprintf(stderr, "Error: could not open log.txt\n");
        exit(1);
    }


    printf("===================================\n");
    printf("Maze Runner Game\n");
    printf("===================================\n");
    Cell maze[FLOORS][WIDTH][LENGTH];
    Flag flag;
    Player players[PLAYERS] = {
        {'A', 0, 6, 12, 0, 6, 12, 0, 0, 0, 100, 0, 0, 0, 0, {0}, {0}, {0}},
        {'B', 0, 9, 8, 0, 9, 8, 3, 0, 0, 100, 0, 0, 0, 0, {0}, {0}, {0}},
        {'C', 0, 9, 16, 0, 9, 16, 1, 0, 0, 100, 0, 0, 0, 0, {0}, {0}, {0}}
    };

    Stairmode initialStairMode = currentMode(0);
    initMaze(maze, &flag, initialStairMode);

    int isRunning = 1, roundCount = 1;

    for (int i = 0; i < PLAYERS; i++) {
        fprintf(logFile, "Player %c starts at [%d,%d,%d]\n",
                players[i].id, players[i].floor, players[i].x, players[i].y);
    }

    while (isRunning) {
        
        printf("\n--------- Round %d --------\n", roundCount);
        
        for (int i = 0; i < PLAYERS; i++) {
            
            printf("\n--------- Player %c's Move -----------\n", players[i].id);

            handleBawanaEffects(&players[i], maze);

            if (players[i].disabledTurns > 0) {
                continue;
            }

            int steps = rollMovementDice();
            int dirRoll = -1;

            if (players[i].inMaze == 0) {
                if (steps == 6) {
                    printf("Player %c is at the starting area and rolls 6 on the movement dice and is placed on [%d,%d,%d] of the maze.\n", players[i].id, players[i].floor, players[i].x, players[i].y);
                    players[i].inMaze = 1;
                    int result = movePlayer(&players[i], 1, maze, initialStairMode, players, flag);
                    if (result == 2) { isRunning = 0; break; }
                } else {
                    printf("Player %c is at the starting area and rolls %d on the movement dice cannot enter the maze.\n", players[i].id, steps);
                }
            } else {
                players[i].turnCount++;
                if (players[i].turnCount % 4 == 0) {
                    dirRoll = rollDirectionDice();
                    if (dirRoll != -1) {
                        players[i].direction = dirRoll;
                        printf("Player %c rolls and %d on the movement dice and %d on the direction dice, changes direction to %s and moves %d cells and is now at [%d,%d,%d].\n",
                               players[i].id, steps, dirRoll, directionToString(players[i].direction), steps, players[i].floor, players[i].x, players[i].y);
                    } else {
                        printf("Player %c rolls and %d on the movement dice and moves %s by %d cells and is now at [%d,%d,%d].\n",
                           players[i].id, steps, directionToString(players[i].direction), steps, players[i].floor, players[i].x, players[i].y);
                    }
                } else {
                     printf("Player %c rolls and %d on the movement dice and moves %s by %d cells and is now at [%d,%d,%d].\n",
                           players[i].id, steps, directionToString(players[i].direction), steps, players[i].floor, players[i].x, players[i].y);
                }
                int result = movePlayer(&players[i], steps, maze, initialStairMode, players, flag);
                if (result == 2) { isRunning = 0; break; }
            }
            
            
            if (players[i].inMaze) {
                 printf("Player %c now has %d movement points.\n", players[i].id, players[i].movePoints);
            }
        }
        roundCount++;
        if (!isRunning) { break; }
    }
    fclose(logFile);


}