/*******************************************************
 File ...... : game.c
 Author .... : Romain VINDERS
 Date ...... : 27/05/2014
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
#include <stdlib.h>
#include <windows.h>
#include "game.h"

/* Dependencies */
#include "collision.h"
#include "game_ai.h"
#include "../render/game_render.h"
#include "../sound.h"

/*-----------------------------------------------------*/

/**  Role : get current keys pressed by user
  *  Output : - */
void game_getUserInput(
    CONFIG *pstParams,  //parameters : state, action, options
    PLAYER *astPlayers  //players information
)
{
    /* variables */
    short i = 0;
    KEYMAP stPressedKeys = {{ 0 }};

    /* avoid key repetitions */
    if( pstParams->pause != 0 )
    {
        pstParams->pause++;
    }

    /* check special keys (exit/pause) */
    stPressedKeys.esc = GetAsyncKeyState( VK_ESCAPE );
    stPressedKeys.pause = GetAsyncKeyState( 0x50 );
    if( stPressedKeys.esc != 0 ) //exit -> ingame menu
    {
        if( pstParams->status > -4 || pstParams->status == -7 )
        {
            render_clear();
            if( pstParams->status == -1 )
            {
                pstParams->status = 1;
                if( pstParams->music != 0 )
                {
                    sfx_playMusic( 0 );
                    sfx_loadMusic( 0, NULL );
                }
            }
            else
            {
                pstParams->status = 2;
                if( pstParams->music != 0 )
                    sfx_playMusic( 0 );
            }
        }
        if( pstParams->code <= 0 )
            pstParams->code = 0;
        else
            pstParams->code = 1;
    }
    else if( stPressedKeys.pause != 0 ) //pause -> toggle state
    {
        if( pstParams->status > -4 && pstParams->status != -1 )
        {
            if( pstParams->status != -3 && pstParams->pause == 0 )
            {
                pstParams->status = -3;
                pstParams->pause = 1;
            }
            else if( pstParams->pause > 8 )
            {
                pstParams->status = -2;
                pstParams->pause = -8;
            }
        }
        if( pstParams->code <= 0 )
            pstParams->code = 0;
        else
            pstParams->code = 1;
    }

    /* check movement keys (if ingame) */
    else if( pstParams->status == -2 )
    {
        /* single player mode */
        if( pstParams->level != 4 )
        {
            stPressedKeys.up[0] = GetKeyState( VK_UP ) & 0x8000;
            stPressedKeys.down[0] = GetKeyState( VK_DOWN ) & 0x8000;
        }
        /* 2 players mode -> check 2 players */
        else
        {
            stPressedKeys.up[0] = GetKeyState( 0x5A ) & 0x8000;
            stPressedKeys.down[0] = GetKeyState( 0x53 ) & 0x8000;
            stPressedKeys.up[1] = GetKeyState( VK_UP ) & 0x8000;
            stPressedKeys.down[1] = GetKeyState( VK_DOWN ) & 0x8000;
            i++;
        }
        /* update all players */
        do
        {
            /* only one arrow pressed */
            if( (stPressedKeys.up[i] ^ stPressedKeys.down[i]) != 0 )
            {
                if( stPressedKeys.up[i] != 0 ) //up arrow pressed
                {
                    astPlayers[i].action[0] = -1;
                    if( astPlayers[i].action[1] > -3 )
                    {
                        astPlayers[i].action[1]--;
                    }
                }
                else //down arrow pressed
                {
                    astPlayers[i].action[0] = 1;
                    if( astPlayers[i].action[1] < 3 )
                    {
                        astPlayers[i].action[1]++;
                    }
                }
            }
            /* idle */
            else
            {
                astPlayers[i].action[0] = 0;
                if( astPlayers[i].action[1] != 0 )
                {
                    if( astPlayers[i].action[1] > 0 )
                    {
                        astPlayers[i].action[1]--;
                    }
                    else
                    {
                        astPlayers[i].action[1]++;
                    }
                }
            }
            i--;
        }
        while( i >= 0 );
    }

    /* check start key */
    else if( pstParams->status == -1 && pstParams->bonus[0] != 0 )
    {
        if( GetAsyncKeyState( 10 ) != 0 || GetAsyncKeyState( 13 ) != 0 )
        {
            pstParams->status = -6;
            pstParams->clock[0] = 0;
        }
    }

    /* cheat code */
    switch( pstParams->code )
    {
        case 0:
            if( GetAsyncKeyState( 0x47 ) != 0 )
                pstParams->code--;
            break;
        case 1:
            if( GetAsyncKeyState( 0x47 ) != 0 )
                pstParams->code++;
            break;
        case -1:
            GetAsyncKeyState( 0x47 );
            GetAsyncKeyState( 0x44 );
            if( GetAsyncKeyState( 0x4F ) != 0 )
                pstParams->code--;
            break;
        case -2:
            GetAsyncKeyState( 0x4F );
            if( GetAsyncKeyState( 0x44 ) != 0 && pstParams->level < 3 )
                pstParams->code = 1;
            break;
        case 2:
            GetAsyncKeyState( 0x47 );
            GetAsyncKeyState( 0x44 );
            if( GetAsyncKeyState( 0x4F ) != 0 )
                pstParams->code++;
            break;
        case 3:
            GetAsyncKeyState( 0x4F );
            if( GetAsyncKeyState( 0x44 ) != 0 )
                pstParams->code = 0;
            break;
    }
}//game_getUserInput

/*-----------------------------------------------------*/

/**  Role : main game logic and execution
  *  Output : - */
void game_run(
    CONFIG *pstParams,  //parameters : state, action, bonus, options
    BALL *pstBall,      //ball information
    PLAYER *astPlayers  //players information
)
{
    /* variable */
    short i = 0;

    /* choose appropriate action */
    switch( pstParams->status )
    {
        /* ingame state -> update */
        case -2:
            /* artificial intelligence */
            if( pstParams->level != 4 )
            {
                ai_run( pstParams, pstBall, astPlayers );
            }

            /* movements and collisions */
            game_movePlayers( pstParams, pstBall, astPlayers );
            i = game_moveBall( pstParams, pstBall, astPlayers );

            /* round has ended */
            if( i > 1 )
            {
                i -= 2;
                astPlayers[i].score += 1;
                pstParams->clock[1] = 0;
                /* end of the game */
                if( astPlayers[i].score >= pstParams->maxScore )
                {
                    pstParams->status = -8 - i;
                    i += 9;
                    /* stop music */
                    if( pstParams->music == 2 )
                        sfx_loadMusic( 0, NULL );
                }
                /* end of the round */
                else
                {
                    pstParams->status = -4 - i;
                    i += 7;
                }
                /* feedback sound */
                if( pstParams->sound != 0 )
                {
                    if( pstParams->level == 4 && i%2 == 0 )
                    {
                        i--;
                    }
                    sfx_playSound( i );
                }
            }
            else
            {
                /* collision sound */
                if( i == 1 && pstParams->sound != 0 )
                    sfx_playSound( 5 );

                /* update current used bonus */
                i = 0;
                do
                {
                    if( astPlayers[i].bonus[0] != 0 )
                    {
                        if( astPlayers[i].bonus[1] <= 1 )
                            astPlayers[i].bonus[0] = 0;
                        else
                            astPlayers[i].bonus[1]--;
                    }
                    i++;
                }
                while( i <= 1 );

                /* check available bonus (expired/collected) */
                if( pstParams->bonus[0] != 0 )
                {
                    game_checkBonus( pstParams, pstBall, astPlayers );
                }
                else
                {
                    /* generate new random bonus */
                    if( pstParams->clock[1] > 8 && rand()%16 == 3 )
                    {
                        game_createBonus( pstParams, pstBall, astPlayers );
                    }
                }

                /* timeout */
                if( pstBall->timeout > 20 )
                {
                    pstParams->status = -6;
                }
            }//else
            break;

    /* new game */
        case -1:
            if( pstParams->bonus[0] == 0 )
            {
                pstParams->bonus[0]++;
                pstParams->clock[0] = 0;
                pstParams->code = 0;
                /* launch music */
                if( pstParams->music != 0 )
                {
                    sfx_loadMusic( 1, pstParams->musicLength );
                    sfx_playMusic( 1 );
                    pstParams->music = 2;
                }
                /* reset players */
                do
                {
                    astPlayers[i].coord = 0;
                    astPlayers[i].offset = 42;
                    astPlayers[i].bonus[0] = 0;
                    astPlayers[i].score = 0;
                    i++;
                }
                while( i <= 1 );
                /* adjust players size */
                switch( pstParams->level )
                {
                    case 3: //hard
                        astPlayers[0].offset = 36;
                    case 1: //easy
                        astPlayers[1].offset = 36;
                        break;
                }
            }
            break;

    /* new round */
        case -6:
            pstParams->status = -7;
            pstParams->bonus[0] = 0;
            pstBall->timeout = 0;
            system( "cls" );
            /* players init */
            do
            {
                astPlayers[i].coord = 0;
                astPlayers[i].speed = 0;
                astPlayers[i].action[0] = 0;
                astPlayers[i].action[1] = 0;
                astPlayers[i].action[2] = 0;
                astPlayers[i].action[3] = 7 - pstParams->level;
                astPlayers[i].bonus[0] = 0;
                i++;
            }
            while( i <= 1 );
            /* ball init */
            game_createBall( pstParams->level, pstBall );
            /* new round sound */
            if( pstParams->sound != 0 )
                sfx_playSound( 4 );
            break;

    /* animation */
        case -7:
            pstParams->bonus[0] += 1;
            break;
    }//switch
}//game_run

/*-----------------------------------------------------*/

/**  Role : add a new collectible bonus
     Bonus : 1 = god mode
             2 = slow down ball (permanent)
             3 = bigger player
             4 = faster player
             5 = slower player
             6 = smaller player
             7 = make opponent become slower
             8 = make opponent become smaller
  *  Output : - */
void game_createBonus(
    CONFIG *pstParams, //parameters
    BALL *pstBall,     //ball information
    PLAYER *astPlayers //players information
)
{
    /* variable */
    short i = 0;

    /* choose side */
    i = 0;                         //easy -> 100% player
    if( pstParams->level > 1 )
    {
        if( pstParams->level == 2 )//medium -> 75% player
        {
            i = rand()%4;
            i /= 3;
        }
        else                       //hard/2p -> 50% each
        {
            i = rand()%4;
            i /= 2;
        }
    }
    if( astPlayers[i].bonus[0] == 0 )
    {
        /* reset timer */
        pstParams->clock[1] = 0;

        /* set vertical position (opposed to player) */
        pstParams->bonus[1] = i;
        if( astPlayers[i].coord > 20 )
        {
            pstParams->bonus[2] = -160 - rand()%48;
        }
        else if( astPlayers[i].coord < -20 )
        {
            pstParams->bonus[2] = 160 + rand()%48;
        }
        else
        {
            pstParams->bonus[2] = 140 + rand()%64;
            if( pstParams->bonus[2]%2 == 1 )
            {
                pstParams->bonus[2] *= -1;
            }
        }

        /* pick random type */
        if( pstParams->level == 4 ) //2 players
        {
            pstParams->bonus[0] = 3 + rand()%6;
        }
        else
        {
            if( i == 0 ) //single player -> difficulty dependant
            {
                switch( pstParams->level )
                {
                    case 2: //medium -> neutral
                        if( pstBall->speedX < 24 )
                        {
                            pstParams->bonus[0] = 3 + rand()%4;
                        }
                        else
                        {
                            pstParams->bonus[0] = 2 + rand()%5;
                        }
                        break;
                    case 3: //hard -> only 3 to 6
                        pstParams->bonus[0] = 3 + rand()%4;
                        break;
                    default: //easy -> higher chance of nice bonus
                        pstParams->bonus[0] = 2 + rand()%25;
                        if( pstParams->bonus[0] > 6 )
                        {
                            pstParams->bonus[0] = 1 + (pstParams->bonus[0]-2)/6;
                        }
                        /* check ball speed (not slowed if min speed) */
                        if( pstParams->bonus[0] == 2 && pstBall->speedX < 20 )
                        {
                            pstParams->bonus[0] = 3 + rand()%2;
                        }
                }//switch
            }
            else //AI -> only 3 to 6
            {
                pstParams->bonus[0] = 3 + rand()%4;
            }
        }//else
    }//if
}//game_createBonus

/*-----------------------------------------------------*/

/**  Role : check available bonus (expired/collected)
     Bonus : 1 = god mode
             2 = slow down ball (permanent)
             3 = bigger player
             4 = faster player
             5 = slower player
             6 = smaller player
             7 = make opponent become slower
             8 = make opponent become smaller
  *  Output : - */
void game_checkBonus(
    CONFIG *pstParams,  //parameters
    BALL *pstBall,      //ball information
    PLAYER *astPlayers  //players information
)
{
    /* variables */
    short sCoord = 0,
          sOffset = 0;
    /* check expiration */
    if( pstParams->clock[1] > 3 )
    {
        pstParams->bonus[0] = 0;
    }
    else
    {
        /* if player on it, assign bonus to player */
        sCoord = astPlayers[pstParams->bonus[1]].coord;
        sOffset = astPlayers[pstParams->bonus[1]].offset;
        if( pstParams->bonus[2] - 10 < sCoord + sOffset
         && pstParams->bonus[2] + 10 > sCoord - sOffset )
        {
            if( pstParams->sound != 0 )
            {
                if( pstParams->bonus[0] > 4 )
                    sfx_playSound( 12 );
                else if( pstParams->bonus[0] == 1 )
                    sfx_playSound( 6 );
                else
                    sfx_playSound( 11 );
            }
            if( pstParams->bonus[0] != 2 )
            {
                if( pstParams->bonus[0] >= 7 )
                {
                    pstParams->bonus[1] ^= 1;
                    pstParams->bonus[0] -= 2;
                }
                astPlayers[pstParams->bonus[1]].bonus[0] = pstParams->bonus[0];
                astPlayers[pstParams->bonus[1]].bonus[1] = 160 + rand()%160;
            }
            else //bonus 2 -> slow down the ball
            {
                pstBall->actionX *= 0.7;
                pstBall->actionY *= 0.7;
                pstBall->speedX = pstBall->actionX;
                pstBall->speedY = pstBall->actionY;
            }
            pstParams->bonus[0] = 0;
            pstParams->clock[1] = 0;
        }
    }
}//game_checkBonus
