/*******************************************************
 File ...... : data_input.c
 Author .... : Romain VINDERS
 Date ...... : 12/05/2014
--------------------------------------------------------
 - get user input : enter a number
********************************************************/
#include <stdio.h>
#include <conio.h>
#include <windows.h>
#include "data_input.h"

/*-----------------------------------------------------*/

/**  Role : get a valid integer number
  *  Output : - */
void input_number(
    short *psValue,  //pointer to value
    short *asLimits, //limits array (min, max, length)
    short sX,        //X position
    short sY         //Y position
)
{
    /* variables */
    HANDLE hConsole = GetStdHandle( STD_OUTPUT_HANDLE );
    COORD coord = { 0 };
    char cInput = 0;
    short i = 0,
          sInputValue = 0,
          sFinished = 0;

    /* clean value zone */
    SetConsoleTextAttribute( hConsole, 0x70 );
    coord.X = sX;
    coord.Y = sY;
    SetConsoleCursorPosition( hConsole, coord );
    printf( "%4c", ' ' );
    /* display previous value from the left side */
    coord.X = sX;
    coord.Y = sY;
    SetConsoleCursorPosition( hConsole, coord );
    if( *psValue != -1 )
    {
        i = printf( "%hd", *psValue );
        sInputValue = *psValue;
    }

    /* empty input buffer */
    while( _kbhit() != 0 )
    {
        _getch();
    }
    /* get input value */
    do
    {
        cInput = _getch();

        /* valid figure */
        if( cInput >= '0' && cInput <= '9' )
        {
            /* below max length -> store and display */
            if( i < asLimits[2] )
            {
                sInputValue = sInputValue*10 + cInput-'0';
                printf( "%c", cInput );
                i++;
            }
        }

        /* special keys */
        else
        {
            switch( cInput )
            {
                /* backspace -> remove last figure */
                case 8:
                    if( i > 0 )
                    {
                        sInputValue = sInputValue/10;
                        i--;
                        printf( "\b \b" );
                    }
                    break;

                /* enter -> check and confirm */
                case 10:
                case 13:
                    if( i > 0 )
                    {
                        /* check limits (min/max) */
                        if( sInputValue < asLimits[0] )
                        {
                            sInputValue = asLimits[0];
                        }
                        else
                        {
                            if( sInputValue > asLimits[1] )
                            {
                                sInputValue = asLimits[1];
                            }
                        }
                        *psValue = sInputValue;
                    }

                /* escape -> cancel */
                case 27:
                    sFinished = 1;
                    break;

                /* invalid input -> empty buffer */
                default: while( _kbhit() != 0 )
                         {
                             _getch();
                         }
            }//switch
        }//else
    }
    while( sFinished == 0 );
}//input_number
