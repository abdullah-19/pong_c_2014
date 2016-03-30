/*******************************************************
 File ...... : menu.h
 Author .... : Romain VINDERS
 Date ...... : 12/05/2014
--------------------------------------------------------
 - initialize window title, size, buffer, position
 - toggle cursor visibility
 - display specific menu
 - execute specific menu and get user input
 - display options
 - execute options and change settings
 - change menu selection
********************************************************/
#ifndef MENU_H
#define MENU_H 1

/* Config inclusion */
#include "config/menu_cfg.h"
#include "config/game_struct.h"

/* Constants */
#define STYLE_WIDTH 670
#define STYLE_HEIGHT 570

/* Prototypes - config and input */
void menu_windowInit();
void menu_adjustCursor( short );
void menu_run( CONFIG* );
short menu_getUserInput( CONFIG*, char*, short, short, short );

/* Prototypes - render */
void menu_setMenu( CONFIG* );
void menu_drawMenu( char*, short );
void menu_drawOptions( CONFIG* );
void menu_moveSelection(short, short, short, char*, char*, short );

#endif
