#include <stdio.h>
#include <stdbool.h>
#include "game.h"
#include "helpers.h"
#include "reachability.h"

int checkReachability(Cell maze[FLOORS][WIDTH][LENGTH], int startF, int startX, int startY, Flag flag) {
    bool visited[FLOORS][WIDTH][LENGTH] = {0};

    // BFS queue
    CellPosition queue[FLOORS * WIDTH * LENGTH];
    int head = 0, tail = 0;

    if (!inBounds(startF, startX, startY) || !inBounds(flag.floor, flag.x, flag.y)) {
        fprintf(stderr, "Start or flag is out of bounds!\n");
        return 0;
    }

    // Start BFS
    queue[tail++] = (CellPosition){startF, startX, startY};
    visited[startF][startX][startY] = true;


    int dx[4] = {-1, 0, 1, 0};
    int dy[4] = {0, 1, 0, -1};

    while (head < tail) {
        CellPosition cur = queue[head++];

        if (cur.f == flag.floor && cur.x == flag.x && cur.y == flag.y) {
            fprintf(logFile, "Flag is reachable\n");
            return 1;
        }

        // 4 directions
        for (int i = 0; i < 4; i++) {
            int nf = cur.f;
            int nx = cur.x + dx[i];
            int ny = cur.y + dy[i];

            if (inBounds(nf, nx, ny) && !visited[nf][nx][ny] && maze[nf][nx][ny].type != 1 && maze[nf][nx][ny].type != 5) {

                visited[nf][nx][ny] = true;
                if (tail < FLOORS * WIDTH * LENGTH)
                    queue[tail++] = (CellPosition){nf, nx, ny};
            }
        }

        // Stairs
        Cell *cell = &maze[cur.f][cur.x][cur.y];
        if (cell->type == 2) {
            for (int i = 0; i < 2; i++) {
                int sf = cell->stairTargetFloors[i];
                int sx = cell->stairTargetXs[i];
                int sy = cell->stairTargetYs[i];

                if (sf >= 0 && sx >= 0 && sy >= 0 && inBounds(sf, sx, sy) && !visited[sf][sx][sy] && maze[sf][sx][sy].type != 1 && maze[sf][sx][sy].type != 5) {

                    visited[sf][sx][sy] = true;
                    if (tail < FLOORS * WIDTH * LENGTH)
                        queue[tail++] = (CellPosition){sf, sx, sy};
                }
            }
        }

        // Poles
        if (cell->type == 3) {
            int pf = cell->targetFloor;
            int px = cell->targetX;
            int py = cell->targetY;

            if (pf >= 0 && px >= 0 && py >= 0 &&
                inBounds(pf, px, py) &&
                !visited[pf][px][py] &&
                maze[pf][px][py].type != 1 && maze[pf][px][py].type != 5) {

                visited[pf][px][py] = true;
                if (tail < FLOORS * WIDTH * LENGTH)
                    queue[tail++] = (CellPosition){pf, px, py};
            }
        }
    }

    fprintf(stderr, "Flag not reachable from [%d,%d,%d]\n", startF, startX, startY);
    return 0;
}
