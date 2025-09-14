# **Maze To Savor!**
Maze to Savor is a multi-player board game written in C, where 3 players navigate through a 3D maze to capture the flag while facing obstacles such as walls, stairs, poles.

## Overview
* A 3 player game
* Goal is to capture tha flag.
* Dice based movement
* Incorporates movement points
* The summary of each game is recorded in `log.txt`
* The errors during the execution are recorded in `errors.txt`

## Maze Design
* 3 Floors
* Block size 4 sq.ft each
* Coordinate format of a cell : *[floor number, width index, length index]*
* Cell starts at *[0,0,0]*

## Movement Rules
* Player rolls movement dice to move. Must roll 6 to enter the maze.
* Direction is determined every 4th round by direction dice.
* Movement occurs only if the full dice value can be moved. No partial movement.

## Obstacles
### Stairs
* Format : *[start floor, start width, start length, end floor, end width, end length]* 
* Can be birectional, upper to lower or lower to upper.

### Poles
* Format : *[start floor, end floor, width, length]*
* Always upper to lower movement

### Walls
* Format : *[floor, start width, start length, end width, end length]*
* Player cannot pass through these.


## Bawana
* Special area where players go when their movement points are not sufficient to make a move.
* 5 special effects from bawana
    + Food poisoning: misser next 3 turns
    + Disoriented: random movement with 50 movement points
    + Triggered: double movement with 50 movement points
    + Happy: gains 200 movement points
    + Random: gains random movement points from 10 to 100

## Inputs
* `stairs.txt`
* `poles.txt`
* `walls.txt`
* `flag.txt`
* `seed.txt`

## Assumptions
* Maximum of two stairs can originate from the same cell; the closest stair to the flag is chosen.
* Infinite loops are detected and the player is moved to their starting cell, keeping their movement points.
* Movement points are capped at a maximum value of 250.
* Walls are straight.
* No stairs or poles originate from or go to the Bawana area or Standing area.
* Stairs that go from floor 0 to floor 2 always cross floor 1 through a void space.
* No stair or pole can go through walls or void space.

## Compile and Run

### Using Bash 
* To compile the program:
```
gcc bawana.c helpers.c initmaze.c players.c game.c main.c reachability.c -o maze_runner
```
* To run the program and redirect errors to errors.txt:
```
./maze_runner 2> errors.txt
```



