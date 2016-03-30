/*******************************************************
 File ...... : game_render.h
 Author .... : Romain VINDERS
 Date ...... : 28/05/2014
--------------------------------------------------------
 - clear screen
 - configure and render a whole frame:
   - instructions frame
   - animation frame
   - pause frame
   - message frame
   - gameplay frame
 - draw subparts (walls,scores,ball,players,bonus,anim)
********************************************************/
#ifndef GAME_RENDER_H
#define GAME_RENDER_H 1

/* Config inclusion */
#include "../config/game_cfg.h"
#include "../config/game_struct.h"

/* Prototypes */
void render_setRendering( CONFIG*, float, BALL*, PLAYER* );
void render_clear();
void render_drawFrame( short, float, CONFIG*, BALL*, PLAYER* );
void render_drawBackground( HDC*, short, short, PLAYER* );
void render_drawBall( HDC*, short, short, short );
void render_drawPlayers( HDC*, short*, short*, short* );
void render_drawBonus( HDC*, short* );
void render_drawStartAnimation( HDC*, short*, float );

#endif
