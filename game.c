#include <stdio.h> 
#include <stdlib.h>
#include "game.h" 
#include "helpers.h"  
#include "initMaze.h"
#include "player.h" 
#include "bawana.h" 
#include "reachability.h"

FILE *logfp = NULL; 


void play(void) {
    int seed = addSEED("seed.txt");
    srand(seed);

    logfp = fopen("log.txt", "a");
    if (!logfp) {
        fprintf(stderr, "Error: could not open log.txt\n");
        exit(1);
    } else {
        fprintf(logfp, "-------------------------------------------\n");
    }

    printf("===================================\n");
    printf("Maze Runner Game\n");
    printf("===================================\n");

    //Initializing play
    Cell maze[FLOORS][WIDTH][LENGTH];
    Flag flag;
    Player players[PLAYERS] = {
        {'A', 0, 6, 12, 0, 6, 12, 0, 0, 0, 100, -1, -1, -1, 0, {0}, {0}, {0}},
        {'B', 0, 9, 8, 0, 9, 8, 3, 0, 0, 100, -1, -1, -1, 0, {0}, {0}, {0}},
        {'C', 0, 9, 16, 0, 9, 16, 1, 0, 0, 100, -1, -1, -1, 0, {0}, {0}, {0}}
    };

    Stairmode stairMode = currentMode(0);
    initMaze(maze, &flag, stairMode);

    //Checking reachability
    int check = 0;
    for (int i = 0; i < PLAYERS; i++) {
        if (!checkReachability(maze, players[i].startFloor, players[i].startX, players[i].startY, flag)) {
            printf("The flag is NOT reachable from any player's start position.\n");
            fprintf(stderr, "The flag is NOT reachable from any player's start position.\n\n");
            return;
        } else {
            check = 1;
        }
    }
    if (check) {
        fprintf(logfp, "Flag is reachable\n");
    }

    // Game loop
    int isRunning = 1, roundCount = 1;

    for (int i = 0; i < PLAYERS; i++) {
        fprintf(logfp, "Player %c starts at [%d,%d,%d]\n",
                players[i].id, players[i].floor, players[i].x, players[i].y);
    }

    while (isRunning) {
        printf("\n--------- Round %d --------\n", roundCount);

        stairMode = currentMode(roundCount);

        for (int i = 0; i < PLAYERS; i++) {
            printf("\n--------- Player %c's Move -----------\n", players[i].id);

            
            handleBawanaEffects(&players[i], maze);

            if (players[i].disabledTurns >= 0) {
                continue;
            }

            int steps = rollMovementDice();
            int result = 0;

            if (players[i].inMaze == 0) {
                if (steps == 6) {
                    result = movePlayer(&players[i],1,maze, stairMode, players, flag);
                    if (result == 2) { //FLag captured
                    isRunning = 0;
                    break;
                }
                    printf("Player %c is at the starting area and rolls 6 on the movement dice and is placed on [%d,%d,%d] of the maze.\n", 
                           players[i].id, players[i].floor, players[i].x, players[i].y);

                    players[i].inMaze = 1;
                } else {
                    printf("Player %c is at the starting area and rolls %d on the movement dice and cannot enter the maze.\n", players[i].id, steps);
                }


            } else {
                players[i].turnCount++;

                result = movePlayer(&players[i], steps, maze, stairMode, players, flag);
                if(result == 0){ // cannot amek move
                    continue;
                }
                if (result == 2) { // Flag captured
                    isRunning = 0;
                    break;
                }
            }
        }
        roundCount++;
    }
    fclose(logfp);
    exit(0);
}
