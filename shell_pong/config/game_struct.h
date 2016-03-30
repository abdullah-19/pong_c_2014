/*******************************************************
 File ...... : game_struct.h
 Author .... : Romain VINDERS
 Date ...... : 22/05/2014
--------------------------------------------------------
 - define global game structure
 - define gameplay structures (players, ball)
 - define misc structures (keymap)
********************************************************/
#ifndef GAME_STRUCT_H
#define GAME_STRUCT_H 1

#define DEBUG 0 //1=dev, 0=release

/* Input structure */
typedef struct
{
    short up[2];    //up arrow (2 players)
    short down[2];  //up arrow (2 players)
    short pause;    //pause key
    short esc;      //escape key
} KEYMAP;

/* Game structures */
typedef struct
{
    short status;           //current status
    short pause;            //pause length
    unsigned short clock[2];//timers (time elapsed, special timer)
    short level;            //mode (difficulty/2 players)
    short maxScore;         //game over
    short sound;            //sound on/off
    short music;            //music on/off
    char musicLength[12];   //music length
    short bonus[3];         //bonus item (type, side, pos)
    short code;             //cheat code
} CONFIG;
typedef struct
{
    short coord;     //Y center position
    short speed;     //Y speed (top->bottom)
    short offset;    //half-length (coord to border)
    short score;     //points
    short action[5]; //direction intended/speed/AI strategy/counter/destination
    short bonus[2];  //current bonus
} PLAYER;
typedef struct
{
    short x;        //X center position
    short y;        //Y center position
    short speedX;   //X speed (left->right)
    short speedY;   //Y speed (top->bottom)
    short actionX;  //X move (left->right)
    short actionY;  //Y move (top->bottom)
    short timeout;
} BALL;

#endif
