/************************************************************
 Project.... : Shell Pong
 Mode....... : console
 OS Support. : Windows NT (2000+)
 Requires... : WinGDI, WinMM
 Author..... : Romain VINDERS
 Project mod : 12/06/2014
 Version.... : 2.0.1
-------------------------------------------------------------
 File name.. : main.c
 File mod... : 28/05/2014
-------------------------------------------------------------
 2.0.0 - refactoring + architecture/loops + new menus/options
 2.0.1 - game logic/render remade (loop/game/coll./AI/bonus)
************************************************************/
//#define MIN_FRAMETIME 17 //max 58.82 FPS (+-60)
//#define MIN_FRAMETIME 12 //max 83.33 FPS (+-80)
#define MIN_FRAMETIME 8 //max 125 FPS
#include <stdlib.h>
#include <time.h>
#include <windows.h>

/* File inclusions */
#include "config/menu_cfg.h"
#include "config/game_struct.h"
#include "menu.h"
#include "game/game.h"
#include "render/game_render.h"
#include "sound.h"
//set debug mode in game_struct.h
#if DEBUG
    #include <stdio.h>
    HANDLE hConsole = { 0 };
    COORD coord = { 0 };
    float fFramerate = 0;
#endif

/*-----------------------------------------------------*/

/**  Roles : menu and game loops, framerate control
  *  Output : 0 (error free) */
int main( int argc, char *argv[] )
{
    /* frame control variables */
    float fPercentWithinTick = 0;
    DWORD dwNextUpdateTick = 0,
          dwNextSecond = 0,
          dwNextFrame = 0,
          dwTickCount = 0;
          #if DEBUG
          hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
          #endif
    /* game variables */
    CONFIG stParams = { 0 };
    BALL stBall = { 0 };
    PLAYER astPlayers[2] = {{ 0 }};

    /* initialize program */
    menu_windowInit();
    srand( time(NULL) );

    /* initialize settings */
    stParams.status = 1;//1=main menu, 2=ingame menu, 0= exit
                        //-1=new game, -2=ingame, -3=pause
                        //-4=round p1, -5=round p2, -6=new round, -7=busy
                        //-8=victory p1, -9=victory p2
    stParams.maxScore = 100;
    stParams.sound = 1;
    stParams.music = 1;

    /* MENU LOOP */
    do
    {
        /* display menu and wait for input */
        menu_setMenu( &stParams );
        menu_run( &stParams );

        /* GAME LOOP */
        if( stParams.status < 0 )
        {
            /* set timers */
            dwTickCount = 0;
            stParams.bonus[0] = 0;
            dwNextFrame = GetTickCount();        //first frame (ms)
            dwNextUpdateTick = dwNextFrame;      //first update (ms)
            dwNextSecond = dwNextFrame + 1025;   //timer
            do
            {
                /* timers update */
                if( dwNextSecond <= dwTickCount )
                {
                    dwNextSecond += 1000;
                    stParams.clock[0]++;
                    if( stParams.status != -3 )
                    {
                        stParams.clock[1]++;
                    }
                    /* refresh music */
                    if( stParams.music != 0 )
                    {
                        sfx_checkMusic( stParams.musicLength );
                    }
                }

                /* user input (no wait) and game logic */
                if( GetTickCount() >= dwNextUpdateTick ) //update 20x/sec
                {
                    game_getUserInput( &stParams, astPlayers );
                    game_run( &stParams, &stBall, astPlayers );
                    dwNextUpdateTick += 50; //next update
                }

                /* framerate limit */
                dwTickCount = GetTickCount();
                if( dwTickCount < dwNextFrame ) //max FPS
                {
                    Sleep( dwNextFrame - dwTickCount ); //wait
                    dwTickCount = dwNextFrame;
                }
                #if DEBUG
                    fFramerate = 1000.0/(MIN_FRAMETIME + (dwTickCount - dwNextFrame) );
                #endif
                dwNextFrame = dwTickCount + MIN_FRAMETIME; //next frame

                /* game display and interpolation */
                fPercentWithinTick = (float)(50 + (DWORD)(dwTickCount - dwNextUpdateTick) )/50.0;
                if( fPercentWithinTick < 0 )
                {
                    fPercentWithinTick = 0;
                }
                render_setRendering( &stParams, fPercentWithinTick, &stBall, astPlayers );

                /* display framerate */
                #if DEBUG
                    coord.X = 5;
                    coord.Y = 2;
                    SetConsoleCursorPosition( hConsole, coord );
                    printf( "%6.2f FPS - %02hu:%02hu", fFramerate,
                            stParams.clock[0]/60, stParams.clock[0]%60 );
                #endif
            }
            while( stParams.status < 0 );
        }//if
    }
    while( stParams.status != 0 );

    /* exit */
    menu_adjustCursor( 1 );
    return 0;
}//main
