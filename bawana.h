#ifndef BAWANA_H
#define BAWANA_H

#include "game.h"

int isInBawana(int f, int x, int y);
void teleportToBawana(Player *p);
void applyBawanaEffect(Player *p, Cell *cell);
void handleBawanaEffects(Player *p, Cell maze[FLOORS][WIDTH][LENGTH]);

#endif
