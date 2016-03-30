/*******************************************************
 File ...... : game_ai.h
 Author .... : Romain VINDERS
 Date ...... : 30/05/2014
--------------------------------------------------------
 - set AI strategy by following action tree
 - delay reaction time
 - return more or less to center
 - follow the ball position with specific offset
 - follow the opposite of the ball position
 - roughly evaluate from far the future ball projection
 - precisely anticipate the future ball position
********************************************************/
#ifndef GAME_AI_H
#define GAME_AI_H 1

/* Config inclusion */
#include "../config/game_cfg.h"
#include "../config/game_struct.h"

/* Prototypes */
void ai_run( CONFIG*, BALL*, PLAYER* );
void ai_delay( short, PLAYER*, BALL* );
void ai_center( short, PLAYER*, BALL* );
void ai_follow( short, short, PLAYER*, BALL* );
void ai_farEval( PLAYER*, BALL* );
void ai_anticipate( short, PLAYER*, BALL* );
short ai_checkBonus( CONFIG*, PLAYER*, BALL* );

#endif
