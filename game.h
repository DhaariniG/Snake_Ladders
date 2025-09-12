#ifndef GAME_H
#define GAME_H

#define FLOORS 3
#define WIDTH 10
#define LENGTH 25
#define PLAYERS 3
#define MAX_MP 250 // New constant for the movement point cap

extern FILE *logFile;  // for writing logs

#define SEED 10

typedef struct {
    int floor;
    int x;
    int y;
} Flag;

typedef struct {
    // Cell types
    // 0=normal, 1=wall, 2=stair, 3=pole, 4=flag, 5=void
    // Bawana-specific types: 6=FoodPoison, 7=Disoriented, 8=Triggered, 9=Happy, 10=Random
    int type;

    // Pole and single-stair targets
    int targetFloor;
    int targetX;
    int targetY;

    // Multi-stair targets (for up/down options)
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
    int x, y;
    int startFloor, startX, startY;
    int direction; // 0=N, 1=E, 2=S, 3=W
    int inMaze; // 1=yes, 0=waiting
    int turnCount;

    int movePoints;
    int disabledTurns; // FoodPoison
    int disorientedTurns; // disoriented
    int triggeredTurns; // triggered

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

// Helper Functions
int inBounds(int f, int x, int y);
void safeSetType(Cell maze[FLOORS][WIDTH][LENGTH], int f, int x, int y, int type);
void safeSetTarget(Cell maze[FLOORS][WIDTH][LENGTH], int f, int x, int y, int targetFloor, int targetX, int targetY, int type);
const char* directionToString(int dir);
int manhattanDistance(int f1, int x1, int y1, int f2, int x2, int y2);

// Dice and Mode
int rollMovementDice();
int rollDirectionDice();
Stairmode currentMode(int roundcount);

// Player history
void addHistory(Player *p);

// Capture and Win
// Display + Movement
const char* directionToString(int dir);
int checkWin(Player *p, Flag flag);
void checkCapture(Player *currentPlayer, Player players[]);

// Maze Setup
void initMaze(Cell maze[FLOORS][WIDTH][LENGTH], Flag *flag, Stairmode stairMode);
void addStandingArea(Cell maze[FLOORS][WIDTH][LENGTH]);
void addVoidSpace(Cell maze[FLOORS][WIDTH][LENGTH]);
void addWalls(Cell maze[FLOORS][WIDTH][LENGTH], const char *filename);
void addFlag(Cell maze[FLOORS][WIDTH][LENGTH], Flag *flag, const char *filename);
void addStairs(Cell maze[FLOORS][WIDTH][LENGTH], const char *filename);
int addStairTarget(Cell maze[FLOORS][WIDTH][LENGTH], int f, int x, int y, int targetFloor, int targetX, int targetY);
int selectBestStair(Cell *cell, Flag flag, Stairmode stairMode, int currentFloor);
void addPoles(Cell maze[FLOORS][WIDTH][LENGTH], const char *filename);



//Bawana
void addConsumables(Cell maze[FLOORS][WIDTH][LENGTH]);

// Movement
void calculateMovementPoints(Player *p, Cell *cell);
int movePlayer(Player *p, int steps, Cell maze[FLOORS][WIDTH][LENGTH], Stairmode stairMode, Player players[], Flag flag);

// Bawana
int isInBawana(int f, int x, int y);
void addBawana(Cell maze[FLOORS][WIDTH][LENGTH]);
void teleportToBawana(Player *p);
void applyBawanaEffect(Player *p, Cell *cell);
void handleBawanaEffects(Player *p, Cell maze[FLOORS][WIDTH][LENGTH]);

// Main Game Loop
void play(void);

#endif
