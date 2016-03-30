/*******************************************************
 File ...... : collision.h
 Author .... : Romain VINDERS
 Date ...... : 09/06/2014
--------------------------------------------------------
 - initialize ball speed and position
 - move players as asked (limited by walls and ball)
 - move ball as asked (limited by players and walls)
 - calculate collisions between ball and walls/players
********************************************************/
#ifndef COLLISION_H
#define COLLISION_H 1

/* Config inclusion */
#include "../config/game_cfg.h"
#include "../config/game_struct.h"

/* Prototypes */
void game_createBall( short, BALL* );
void game_movePlayers( CONFIG*, BALL*, PLAYER* );
short game_moveBall( CONFIG*, BALL*, PLAYER* );
short game_setBallCollision( BALL*, PLAYER*, float* );
void game_prepareCollision( BALL*, PLAYER* );

#endif
