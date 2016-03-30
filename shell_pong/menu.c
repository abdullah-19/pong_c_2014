/*******************************************************
 File ...... : menu.c
 Author .... : Romain VINDERS
 Date ...... : 22/05/2014
--------------------------------------------------------
 - initialize window title, size, buffer, position
 - toggle cursor visibility
 - display specific menu
 - execute specific menu and get user input
 - display options
 - execute options and change settings
 - change menu selection
********************************************************/
#define _WIN32_WINNT 0x0500 //for GetConsoleWindow()
#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include <conio.h>
#include <windows.h>
#include "menu.h"

/* Dependencies */
#include "data_input.h"
#include "sound.h"

/*-----------------------------------------------------*/

/**  Role : window setup/initialization
  *  Output : - */
void menu_windowInit()
{
    /* window variables */
    HWND window = GetConsoleWindow();
    COORD bufferSize = { 80, 44 }; //width, max height
    short sXOffset = ( GetSystemMetrics( SM_CXSCREEN ) - STYLE_WIDTH ) / 2;
    short sYOffset = ( GetSystemMetrics( SM_CYSCREEN ) - STYLE_HEIGHT ) / 2;

    /* display buffer config */
    SetConsoleScreenBufferSize( GetStdHandle( STD_OUTPUT_HANDLE ), bufferSize );
    system( "color 07" );       //black background
    setlocale( LC_CTYPE, "C" ); //C standard keys

    /* window config */
    MoveWindow( window, sXOffset, sYOffset, STYLE_WIDTH, STYLE_HEIGHT, 1 );
    SetConsoleTitle( MAIN_TITLE );
    menu_adjustCursor( 0 );

    /* initialize sound effects */
    sfx_playSound( 0 );
}//menu_windowInit

/*-----------------------------------------------------*/

/**  Role : display or hide console cursor
  *  Output : - */
void menu_adjustCursor(
    short sDisplay  //0=hide, 1=display
)
{
    HANDLE hConsole = GetStdHandle( STD_OUTPUT_HANDLE );
    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo( hConsole, &cursorInfo );
    cursorInfo.bVisible = sDisplay;
    SetConsoleCursorInfo( hConsole, &cursorInfo);
}//menu_adjustCursor

/*-----------------------------------------------------*/

/**  Role : set the right input selection depending on menu type
  *  Output : - */
void menu_run(
    CONFIG *pstParams  //parameters : menu type, action, options
)
{
    /* variables */
    short sChoice = 0;

    /* adjust settings depending on menu type */
    switch( pstParams->status )
    {
        case 2:
        {
            char aacGameMenu[3][MENU_MAX_LENGTH] = { MENU_GAME, MENU_OPTIONS };
            sChoice = menu_getUserInput( pstParams, &aacGameMenu[0][0], 3, 0, MENU_MAX_LENGTH );
            /* interpret choice */
            switch( sChoice )
            {
                case 0: pstParams->status = 4;
                    break;
                case -1:
                    if( pstParams->music == 2 )
                        sfx_loadMusic( 0, NULL );
                    pstParams->status = 0;
                    system( "cls" );
                    break;
                case 2:
                    if( pstParams->music == 2 )
                    {
                        sfx_loadMusic( 0, NULL );
                        pstParams->music = 1;
                    }
                    if( pstParams->sound != 0 )
                        sfx_playSound( 3 );
                    pstParams->status = 1;
                    pstParams->pause = 0;
                    break;
                case 1:
                    pstParams->status = -2;
                    pstParams->pause = 0;
                    pstParams->bonus[0] = 0;
                    system( "cls" );
                    /* refresh keymap */
                    GetAsyncKeyState( VK_ESCAPE );
                    GetAsyncKeyState( 0x50 );
                    GetAsyncKeyState( 0x47 );
                    /* restart music */
                    if( pstParams->music != 0 )
                    {
                        if( pstParams->music == 1 )
                        {
                            sfx_loadMusic( 1, pstParams->musicLength );
                            pstParams->music = 2;
                            sfx_playMusic( 1 );
                        }
                        else
                        {
                            sfx_playMusic( 2 );
                        }
                    }
                    break;
            }//switch
            break;
        }
        case 3:
        case 4:
        {
            short sSelection = 1,
                  asInputLimits[3] = { 1, 9999, 4 };
            char aacOptionsMenu[OPTIONS_ITEMS][17] = { OPTION_LABELS, MENU_BACK },
                 aacBooleans[2][4] = { MENU_BOOLEANS };
            sprintf( aacOptionsMenu[0], "%s %3s", aacOptionsMenu[0], OPTION_NAME1 );
            sprintf( aacOptionsMenu[1], "%s %3s", aacOptionsMenu[1], OPTION_NAME1B );
            sprintf( aacOptionsMenu[2], "%s %4hd", aacOptionsMenu[2], OPTION_NAME2 );
            do
            {
                sChoice = menu_getUserInput( pstParams, &aacOptionsMenu[0][0], OPTIONS_ITEMS, sSelection, 17 );
                switch( sChoice )
                {
                    case 1:
                        sSelection = 1;
                        pstParams->sound ^= 1;
                        aacOptionsMenu[0][13] = OPTION_NAME1[0];
                        aacOptionsMenu[0][14] = OPTION_NAME1[1];
                        aacOptionsMenu[0][15] = OPTION_NAME1[2];
                        menu_moveSelection( 1, 30, 10, aacOptionsMenu[0], "", 17 );
                        break;
                    case 2:
                        sSelection = 2;
                        pstParams->music ^= 1;
                        aacOptionsMenu[1][13] = OPTION_NAME1B[0];
                        aacOptionsMenu[1][14] = OPTION_NAME1B[1];
                        aacOptionsMenu[1][15] = OPTION_NAME1B[2];
                        menu_moveSelection( 1, 30, 12, aacOptionsMenu[1], "", 17 );
                        break;
                    case 3:
                        sSelection = 3;
                        menu_adjustCursor( 1 );
                        input_number( &(pstParams->maxScore), asInputLimits, 43, 14 );
                        menu_adjustCursor( 0 );
                        aacOptionsMenu[2][11] = 0;
                        sprintf( aacOptionsMenu[2], "%s%4hd", aacOptionsMenu[2], OPTION_NAME2 );
                        menu_moveSelection( 1, 30, 14, aacOptionsMenu[2], "", 17 );
                        if( pstParams->sound != 0 )
                            sfx_playSound( 2 );
                        break;
                    case -1:
                    case 0:
                        if( pstParams->sound != 0 )
                            sfx_playSound( 3 );
                        pstParams->status -= 2;
                    break;
                }//switch
            }
            while( sChoice > 0 );
            break;
        }
        default:
        {
            char aacMainMenu[MENU_ITEMS][MENU_MAX_LENGTH] = { MENU_LABELS, MENU_OPTIONS };
            sChoice = menu_getUserInput( pstParams, &aacMainMenu[0][0], MENU_ITEMS, 0, MENU_MAX_LENGTH );
            /* interpret choice */
            switch( sChoice )
            {
                case 0: pstParams->status = 3;
                    break;
                case -1:
                    pstParams->status = 0;
                    system( "cls" );
                    break;
                case 1:
                case 2:
                case 3:
                case 4:
                    pstParams->status = -1;
                    pstParams->level = sChoice;
                    system( "cls" );
                    /* refresh keymap */
                    GetAsyncKeyState( 10 );
                    GetAsyncKeyState( 13 );
                    GetAsyncKeyState( VK_ESCAPE );
                    GetAsyncKeyState( 0x50 );
                    GetAsyncKeyState( 0x47 );
                    break;
            }//switch
        }
    }//switch
}//menu_run

/*-----------------------------------------------------*/

/**  Role : get user input for menus
  *  Output : choice number */
short menu_getUserInput(
    CONFIG *pstParams,   //parameters
    char *pcMenu,        //menu labels
    short sMaxSelect,    //max selection
    short sDirectInput,  //0=direct input, >0=last position
    short sWidth         //element width
)
{
    /* variables */
    char acPrefix[5] = { 0 };
    short sInput = 0,
          sChoice = -2,
          sSelection = 1;

    /* prepare prefix and selection */
    acPrefix[1] = ' ';
    acPrefix[2] = '-';
    acPrefix[3] = ' ';
    if( sDirectInput > 0 )
    {
        sSelection = sDirectInput;
    }

    /* force user input */
    do
    {
        /* empty input buffer */
        while( _kbhit() != 0 )
        {
            _getch();
        }

        /* wait for input */
        sInput = _getch();
        switch( sInput )
        {
            /* ESC -> exit */
            case 27:
                sChoice = -1;
                break;

            /* ENTER -> select */
            case 10:
            case 13:
                if( pstParams->sound != 0 )
                    sfx_playSound( 2 );
                if( sSelection == sMaxSelect )
                {
                    sChoice = 0;
                }
                else
                {
                    sChoice = sSelection;
                }
                break;

            /* arrows -> move selection */
            case 224:
                sInput = _getch();
                if( sDirectInput == 0 )
                {
                    if( sSelection == sMaxSelect )
                    {
                        acPrefix[0] = '0';
                    }
                    else
                    {
                        acPrefix[0] = sSelection + '0';
                    }
                }
                switch( sInput )
                {
                    case 72: //UP arrow -> move up
                        menu_moveSelection( 0, 30, 8 + sSelection*2,
                                            pcMenu + (sSelection-1)*sWidth,
                                            acPrefix, 17 );
                        sSelection--;
                        if( sSelection < 1 )
                        {
                            sSelection = sMaxSelect;
                        }
                        break;
                    case 80: //DOWN arrow -> move down
                        menu_moveSelection( 0, 30, 8 + sSelection*2,
                                            pcMenu + (sSelection-1)*sWidth,
                                            acPrefix, 17 );
                        sSelection++;
                        if( sSelection > sMaxSelect )
                        {
                            sSelection = 1;
                        }
                        break;
                    default: sInput = 0;
                }
                /* new selection */
                if( sInput != 0 )
                {
                    if( pstParams->sound != 0 )
                        sfx_playSound( 1 );
                    if( sDirectInput == 0 )
                    {
                        if( sSelection == sMaxSelect )
                        {
                            acPrefix[0] = '0';
                        }
                        else
                        {
                            acPrefix[0] = sSelection + '0';
                        }
                    }
                    menu_moveSelection( 1, 30, 8 + sSelection*2,
                                        pcMenu + (sSelection-1)*sWidth,
                                        acPrefix, 17 );
                }
                break;

            /* direct input -> select */
            default:
                if( sDirectInput == 0 && sInput >= '0' && sInput < '0' + sMaxSelect )
                {
                    if( pstParams->sound != 0 )
                        sfx_playSound( 2 );
                    sChoice = sInput - '0';
                }
        }//switch
    }
    while( sChoice == -2 );
    return sChoice;
}
/*-----------------------------------------------------*/

/**  Role : set the right menu to display
  *  Output : - */
void menu_setMenu(
    CONFIG *pstParams  //parameters : menu type, action, options
)
{
    /* set the appropriate menu */
    switch( pstParams->status )
    {
        case 2:
        {
            char aacGameMenu[3][MENU_MAX_LENGTH] = { MENU_GAME, MENU_OPTIONS };
            menu_drawMenu( &aacGameMenu[0][0], 3 );
            break;
        }
        case 3:
        case 4:
        {
            menu_drawOptions( pstParams );
            break;
        }        default:
        {
            char aacMainMenu[MENU_ITEMS][MENU_MAX_LENGTH] = { MENU_LABELS, MENU_OPTIONS };
            menu_drawMenu( &aacMainMenu[0][0], MENU_ITEMS );
        }
    }//switch
}//menu_setMenu

/*-----------------------------------------------------*/

/**  Role : display a whole menu on the screen
  *  Output : - */
void menu_drawMenu(
    char *pcMenu,       //menu labels
    short sItemsNumber  //items number
)
{
    /* menu variables */
    HANDLE hConsole = GetStdHandle( STD_OUTPUT_HANDLE );
    COORD coord = { 0 };
    short i = 0;
    char acLine[47] = { 0 };

    /* line format */
    while( i < 46 ) {
        acLine[i] = 0xC4;
        i++;
    }

    /* refresh and display menu title */
    system( "cls" );
    SetConsoleTextAttribute( hConsole, 0x70 );
    printf( "\n%112c%s%113c\n", ' ', MAIN_TITLE, ' ' );
    SetConsoleTextAttribute( hConsole, 0x07 );

    /* display menu labels */
    printf( "\n%21cBienvenue. Veuillez faire votre choix.\n\n", ' ' );
    printf( "%16c\xDA%s\xBF\n%16c\xB3%46c\xB3\n", ' ', acLine, ' ', ' ' );
    for( i = 1; i <= sItemsNumber; i++ )
    {
        printf( "%16c\xB3%13c", ' ', ' ' ); //left border
        /* number and label */
        if( i == sItemsNumber )
        {
            printf( "  0 - " );
        }
        else
        {
            printf( "  %hd - ", i );
        }
        printf( "%s", pcMenu );

        /* normalize width */
        coord.X = 63;
        coord.Y = 8 + i*2;
        SetConsoleCursorPosition( hConsole, coord );
        printf( "\xB3\n%16c\xB3%46c\xB3\n", ' ', ' ' ); //right border/next line
        /* next label */
        pcMenu += MENU_MAX_LENGTH;
    }
    printf( "%16c\xC0%s\xD9\n", ' ', acLine );
    pcMenu -= sItemsNumber*MENU_MAX_LENGTH;

    /* usable commands list */
    SetConsoleTextAttribute( hConsole, 0x0F );
    printf( "\n%8c0-%d \x18\x19  ", ' ', sItemsNumber - 1 );
    SetConsoleTextAttribute( hConsole, 0x07 );
    printf( MENU_MOVE );
    SetConsoleTextAttribute( hConsole, 0x0F );
    printf( "%8cENTER  ", ' ' );
    SetConsoleTextAttribute( hConsole, 0x07 );
    printf( MENU_SELECT );
    SetConsoleTextAttribute( hConsole, 0x0F );
    printf( "%8cESC  ", ' ' );
    SetConsoleTextAttribute( hConsole, 0x07 );
    printf( MENU_END );

    /* first label selection */
    menu_moveSelection( 1, 30, 10, pcMenu, "1 - ", 17 );
}//menu_drawMenu

/*-----------------------------------------------------*/

/**  Role : display options menu
  *  Output : - */
void menu_drawOptions(
    CONFIG *pstParams  //parameters
)
{
    /* menu variables */
    HANDLE hConsole = GetStdHandle( STD_OUTPUT_HANDLE );
    COORD coord = { 0 };
    short i = 0;
    char aacOptionsMenu[OPTIONS_ITEMS][MENU_MAX_LENGTH] = { OPTION_LABELS, MENU_BACK },
         aacBooleans[2][4] = { MENU_BOOLEANS },
         acSelected[20] = { 0 },
         acLine[47] = { 0 };

    /* line format */
    while( i < 46 ) {
        acLine[i] = 0xC4;
        i++;
    }

    /* refresh and display options title */
    system( "cls" );
    SetConsoleTextAttribute( hConsole, 0x70 );
    printf( "\n%107c%s - %s%108c\n", ' ', MAIN_TITLE, MENU_OPTIONS, ' ' );
    SetConsoleTextAttribute( hConsole, 0x07 );
    if( pstParams->music == 2 )
    {
        sfx_loadMusic( 0, NULL );
        pstParams->music = 1;
    }

    /* display menu labels */
    printf( "\n%24cModifiez les options ci-dessous.\n\n", ' ' );
    printf( "%16c\xDA%s\xBF\n%16c\xB3%46c\xB3\n", ' ', acLine, ' ', ' ' );
    for( i = 0; i < OPTIONS_ITEMS; i++ )
    {
        printf( "%16c\xB3%15c", ' ', ' ' ); //left border
        /* label */
        printf( "%s", aacOptionsMenu[i] );
        /* normalize width */
        coord.Y = 10 + i*2;
        switch( i )
        {
            case 0:
                coord.X = 45;
                SetConsoleCursorPosition( hConsole, coord );
                printf( OPTION_TYPE1, OPTION_NAME1 );
                break;
            case 1:
                coord.X = 45;
                SetConsoleCursorPosition( hConsole, coord );
                printf( OPTION_TYPE1, OPTION_NAME1B );
                break;
            case 2:
                coord.X = 43;
                SetConsoleCursorPosition( hConsole, coord );
                printf( OPTION_TYPE2, OPTION_NAME2 );
                break;
            default:
                coord.X = 48;
                SetConsoleCursorPosition( hConsole, coord );
        }
        printf( "%15c\xB3\n%16c\xB3%46c\xB3\n", ' ', ' ', ' ' ); //right border/next line
    }
    printf( "%16c\xC0%s\xD9\n", ' ', acLine );

    /* usable commands list */
    SetConsoleTextAttribute( hConsole, 0x0F );
    printf( "\n%12c\x18\x19  ", ' ' );
    SetConsoleTextAttribute( hConsole, 0x07 );
    printf( MENU_MOVE );
    SetConsoleTextAttribute( hConsole, 0x0F );
    printf( "%8cENTER  ", ' ' );
    SetConsoleTextAttribute( hConsole, 0x07 );
    printf( MENU_SELECT );
    SetConsoleTextAttribute( hConsole, 0x0F );
    printf( "%8cESC  ", ' ' );
    SetConsoleTextAttribute( hConsole, 0x07 );
    printf( MENU_BACK );

    /* selection */
    sprintf( acSelected, "%s %3s", aacOptionsMenu[0], OPTION_NAME1 );
    menu_moveSelection( 1, 30, 10, acSelected, "", 17 );
}//menu_drawOptions

/*-----------------------------------------------------*/

/**  Role : visually select/deselect a label
  *  Output : - */
void menu_moveSelection(
    short sMode,        //mode (0=deselect, 1=select)
    short sXCoord,      //X coord
    short sYCoord,      //Y coord
    char *acLabel,      //displayed label
    char *acPrefix,     //displayed prefix (optional)
    short sWidth        //selection width
)
{
    /* variables */
    HANDLE hConsole = GetStdHandle( STD_OUTPUT_HANDLE );
    COORD coord = { 0 };
    short i = 0;
    char acBlank[50] = { 0 };

    /* position */
    coord.X = sXCoord;
    coord.Y = sYCoord;
    SetConsoleCursorPosition( hConsole, coord );

    /* préparation de bloc d'ajustement de longueur */
    i = sWidth - ( 1 + strlen( acPrefix ) + strlen( acLabel ) );
    if( i >= 0 )
    {
        acBlank[i] = 0;
        while( i > 0 )
        {
            i--;
            acBlank[i] = ' ';
        }
    }

    /* update selection */
    if( sMode == 1 )
    {
        SetConsoleTextAttribute( hConsole, 0x70 );
        printf( "  %s%s%s  ", acPrefix, acLabel, acBlank );
        SetConsoleTextAttribute( hConsole, 0x07 );
        printf( " " ); //to avoid having 1px offset
    }
    else //désélection
    {
        printf( "  %s%s%s  ", acPrefix, acLabel, acBlank );
    }
}//menu_moveSelection

