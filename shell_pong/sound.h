/*******************************************************
 File ...... : sound.h
 Author .... : Romain VINDERS
 Date ...... : 27/05/2014
--------------------------------------------------------
 - load, play, stop music
 - check and restart music
 - play specific sound effect
********************************************************/
#ifndef SOUND_H
#define SOUND_H 1

/* Prototypes */
void sfx_playSound( short );
void sfx_loadMusic( short, char* );
void sfx_checkMusic( char* );
void sfx_playMusic( short );

#endif
