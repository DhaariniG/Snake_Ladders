#include <stdio.h>   
#include <stdlib.h>  
#include "game.h"    
#include "helpers.h"   
#include "initMaze.h"
#include "bawana.h"

void addStandingArea(Cell maze[FLOORS][WIDTH][LENGTH]) {
    for (int i = WIDTH - 4; i < WIDTH; i++) {
        for (int j = 8; j < 16; j++) {
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

Stairmode currentMode(int roundcount) {
    static Stairmode mode = stairs_bi;
    if (roundcount % 5 == 1) {
        mode = (Stairmode)(rand() % 3);
    }
    return mode;
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
