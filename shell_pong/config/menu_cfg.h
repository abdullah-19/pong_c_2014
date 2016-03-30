/*******************************************************
 File ...... : menu_cfg.h
 Author .... : Romain VINDERS
 Date ...... : 22/05/2014
--------------------------------------------------------
 - define menu constants and labels
********************************************************/
#ifndef MENU_CFG_H
#define MENU_CFG_H 1

/* Constants */
#define MAIN_TITLE "Shell Pong V2.0"

/* Constants - basic labels */
#define MENU_GAME "Reprendre","Arr\x88t partie"
#define MENU_BACK "Retour"
#define MENU_OPTIONS "Options"
#define MENU_END "Quitter"
#define MENU_BOOLEANS "OFF","ON "
#define MENU_MOVE "Naviguer"
#define MENU_SELECT "S\x82lectionner"

/* Constants - custom labels */
#define MENU_MAX_LENGTH 14
#define MENU_ITEMS 5    //custom number + 1 (options)
#define MENU_LABELS "Mode facile","Mode v\x82t\x82ran","Mode expert","Deux joueurs"
#define OPTIONS_ITEMS 4 //custom number + 1 (back)
#define OPTION_LABELS "Effets audio","Musique fond","Victoire :"
#define OPTION_TYPE1 "%3s"
#define OPTION_NAME1 aacBooleans[pstParams->sound]
#define OPTION_NAME1B aacBooleans[pstParams->music]
#define OPTION_TYPE2 "%4hd "
#define OPTION_NAME2 pstParams->maxScore

#endif
