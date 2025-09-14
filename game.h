#ifndef GAME_H
#define GAME_H

#include <stdio.h>
#define FLOORS 3
#define WIDTH 10
#define LENGTH 25
#define PLAYERS 3
#define MAX_MP 250 

extern FILE *logfp;  

#define SEED 10

typedef struct {
    int floor;
    int x;
    int y;
} Flag;

typedef struct {
    int type;
    // 0=normal, 1=wall, 2=stair, 3=pole, 4=flag, 5=void
    // Bawana-specific types: 6=FoodPoison, 7=Disoriented, 8=Triggered, 9=Happy, 10=Random
    
    
    int targetFloor;
    int targetX;
    int targetY;

    // For Multi-stair
    int numStairs;
    int stairTargetFloors[2];
    int stairTargetXs[2];
    int stairTargetYs[2];

    // Consumable effects
    int consumable;
    int bonusAdd;
    int bonusMul;

    int bawanaValue; // For random Bawana cells
} Cell;

typedef struct {
    char id; // A,B,C
    int floor;
    int x;
    int y;
    int startFloor;
    int startX;
    int startY;
    int direction; // 0=N, 1=E, 2=S, 3=W
    int inMaze; // 1=yes, 0=waiting
    int turnCount;

    int movePoints;
    int disabledTurns; // FoodPoison
    int disorientedTurns; // disoriented
    int triggeredTurns; // triggered

    //to detect loop
    int history;
    int lastF[10];
    int lastX[10];
    int lastY[10];
} Player;

typedef enum {
    stairs_bi = 0,
    stairs_up = 1,
    stairs_down = 2,
} Stairmode;

void play(void);

#endif
