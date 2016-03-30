/*******************************************************
 File ...... : game.h
 Author .... : Romain VINDERS
 Date ...... : 12/05/2014
--------------------------------------------------------
 - get user input (keyboard)
 - create and manage game modes
    - initialize new game
    - start new round
    - start animation
    - manage current game and set victory/defeat
 - create random bonus
 - check if players have catched a bonus
********************************************************/
#ifndef GAME_H
#define GAME_H 1

/* Config inclusion */
#include "../config/game_cfg.h"
#include "../config/game_struct.h"

/* Prototypes */
void game_getUserInput( CONFIG*, PLAYER* );
void game_run( CONFIG*, BALL*, PLAYER* );
void game_createBonus( CONFIG*, BALL*, PLAYER* );
void game_checkBonus( CONFIG*, BALL*, PLAYER* );

#endif
