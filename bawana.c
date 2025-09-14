#include <stdio.h>   
#include <stdlib.h>  
#include "game.h"    
#include "helpers.h"   
#include "bawana.h"


int isInBawana(int f, int x, int y) {
    return f == 0 && x > 6 && x <= 9 && y > 20 && y <= 24;
}

void teleportToBawana(Player *p) {
    int count = 0;
    int bawanaCellsX[12], bawanaCellsY[12];
    for (int x = 7; x <= 9; x++) {
        for (int y = 21; y <= 24; y++) {
            if (!inBounds(0, x, y)) {
                continue;
            }
            bawanaCellsX[count] = x;
            bawanaCellsY[count] = y;
            count++;
        }
    }
    int choice = rand() % count;
    p->floor = 0;
    p->x = bawanaCellsX[choice];
    p->y = bawanaCellsY[choice];
}

void applyBawanaEffect(Player *p, Cell *cell) {
    switch (cell->type) {
        case 6:
            printf("Player %c eats from Bawana and have a bad case of food poisoning. Will need three rounds to recover.\n", p->id);
            p->disabledTurns = 3;
            break;
        case 7:
            printf("Player %c eats from Bawana and is disoriented and is placed at the entrance of Bawana with 50 movement points.\n", p->id);
            p->movePoints = 50;
            p->disorientedTurns = 4;

            break;
        case 8:
            printf("Player %c eats from Bawana and is triggered due to bad quality of food. Player %c is placed at the entrance of Bawana with 50 movement points.\n", p->id, p->id);
            p->movePoints = 50;
            p->triggeredTurns = 4;
            break;
        case 9:
            printf("Player %c eats from Bawana and is happy. Player %c is placed at the entrance of Bawana with 200 movement points.\n", p->id, p->id);
            p->movePoints = 200;
            break;
        case 10:
            printf("Player %c eats from Bawana and earns %d movement points and is placed at the [0,9,19].\n", p->id, cell->bawanaValue);
            p->movePoints = cell->bawanaValue;
            break;
    }

    
    if (p->movePoints > MAX_MP) {
        p->movePoints = MAX_MP;
    }

   
    if (cell->type >= 7 && cell->type <= 10) {
        p->x = 9;
        p->y = 19;
        p->direction = 0;
    }
}

void handleBawanaEffects(Player *p, Cell maze[FLOORS][WIDTH][LENGTH]) {
    if (p->disabledTurns >= 1) {
        // still food poisoned
        printf("Player %c is still food poisoned and misses the turn.\n", p->id);
        p->disabledTurns--;
        return;

    }
    //recovered from food poison
    else if (p->disabledTurns == 0) {
            teleportToBawana(p);
            Cell *bawanaCell = &maze[p->floor][p->x][p->y];

            printf("Player %c is now fit to proceed from the food poisoning episode and now placed on a %d and the effects take place.\n",p->id, bawanaCell->type);
            applyBawanaEffect(p, bawanaCell);
            //recovered but again got food poisoned
            if(p->disabledTurns >=1){
                return;
            }
            p->disabledTurns = -1;
            
        }
        
    // still disoriented
    if (p->disorientedTurns > 0) {
        p->direction = rand() % 4;
        p->disorientedTurns--;
    }

    if (p->disorientedTurns == 0) {
        printf("Player %c has recovered from disorientation\n", p->id);
        p->disorientedTurns = -1;
    }

    //still triggered
    if (p->triggeredTurns > 0) {
        p->triggeredTurns--;
    }
}
