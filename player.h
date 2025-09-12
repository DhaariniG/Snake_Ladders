#ifndef PLAYER_H
#define PLAYER_H

#include "game.h"

void addHistory(Player *p);
int checkWin(Player *p, Flag flag);
void checkCapture(Player *currentPlayer, Player players[]);
int movePlayer(Player *p, int steps, Cell maze[FLOORS][WIDTH][LENGTH], Stairmode stairMode, Player players[], Flag flag);

#endif
