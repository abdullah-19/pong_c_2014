/*******************************************************
 File ...... : sound.c
 Author .... : Romain VINDERS
 Date ...... : 27/05/2014
--------------------------------------------------------
 - load, play, stop music
 - check and restart music
 - play specific sound effect
********************************************************/
#include <windows.h>
#include <string.h>
#include "sound.h"

/*-----------------------------------------------------*/

/**  Role : play a specific sound
            0 = start
            1 = menu move
            2 = menu enter
            3 = menu back
            4 = round start
            5 = collision
            6 = god
            7 = round won
            8 = round losed
            9 = game won
            10 = game losed
            11 = bonus picked
            12 = malus picked
  *  Output : - */
void sfx_playSound(
    short sSound    //sound number
)
{
    /* variable */
    char acFile[11] = "sfx/01.wav";

    /* choose destination */
    if( sSound < 10 )
    {
        acFile[5] = sSound + '0';
    }
    else
    {
        acFile[4] = (sSound/10) + '0';
        acFile[5] = (sSound%10) + '0';
    }
    /* play sound */
    if( sSound == 5 )
        PlaySound( acFile, NULL, SND_ASYNC|SND_NOSTOP );
    else
        PlaySound( acFile, NULL, SND_ASYNC );
}//sfx_playSound

/*-----------------------------------------------------*/

/**  Role : load/close music file
  *  Output : - */
void sfx_loadMusic(
    short sStatus,       //open=1, close=0
    char *acMusicLength //music length
)
{
    /* load music file */
    if( sStatus != 0 )
    {
        mciSendString( "open ./sfx/music.mp3 alias music", NULL, 0, NULL );
        mciSendString( "setaudio music volume to 0", NULL, 0, NULL );
        /* get music length (if not already known) */
        if( acMusicLength[0] == 0 )
        {
            mciSendString( "status music length", acMusicLength, 12, 0 );
        }
    }
    /* close file */
    else
    {
        mciSendString( "close music", NULL, 0, NULL );
    }
}//sfx_loadMusic

/*-----------------------------------------------------*/

/**  Role : check music status and reload
  *  Output : - */
void sfx_checkMusic(
    char *acMusicLength
)
{
    /* variable */
    char acMusicStatus[12] = { 0 };

    /* get current position and compare with length */
    mciSendString( "status music position", acMusicStatus, 12, 0 );
    if( strcmp( acMusicLength, acMusicStatus ) == 0 )
    {
        mciSendString( "play music from 0", NULL, 0, NULL ); //reload
    }
}//sfx_checkMusic

/*-----------------------------------------------------*/

/**  Role : play/stop music
  *  Output : - */
void sfx_playMusic(
    short sStatus   //fadein=2, on=1, off=0
)
{
    /* fade in mode (to avoid pops) */
    if( sStatus == 2 )
    {
        /* variables */
        char acFadeIn[29] = "setaudio music volume to 1";
        short i = 1;
        acFadeIn[28] = 0;
        acFadeIn[27] = 0;
        acFadeIn[26] = 0;

        /* play and fade in (0 to 600) */
        mciSendString( "play music", NULL, 0, NULL );
        Sleep( 100 ); //wait for 1 period
        do
        {
            if( i >= 10 )
            {
                if( i >= 100 )
                {
                    acFadeIn[25] = i/100 + '0';
                    acFadeIn[26] = (i/10)%10 + '0';
                    acFadeIn[27] = i%10 + '0';
                }
                else
                {
                    acFadeIn[25] = i/10 + '0';
                    acFadeIn[26] = i%10 + '0';
                }
            }
            else
            {
                acFadeIn[25] = i + '0';
                Sleep( 3 );
            }
            mciSendString( acFadeIn, NULL, 0, NULL );
            i++;
        }
        while( i < 500 );
    }
    /* simple play */
    else if( sStatus == 1 )
    {
        mciSendString( "setaudio music volume to 500", NULL, 0, NULL );
        mciSendString( "play music", NULL, 0, NULL );
    }
    /* stop */
    else
    {
        mciSendString( "stop music", NULL, 0, NULL );
        mciSendString( "setaudio music volume to 0", NULL, 0, NULL );
    }
}//sfx_playMusic
