/*******************************************************
 File ...... : game_render.c
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
#define _WIN32_WINNT 0x0500 //for GetConsoleWindow()
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <conio.h>
#include <windows.h>
#include "game_render.h"

/*-----------------------------------------------------*/

/**  Role : prepare frame rendering depending on status
  *  Output : - */
void render_setRendering(
    CONFIG *pstParams,    //parameters : status, timers, level, bonus
    float fPercentUpdate, //interpolation
    BALL *pstBall,        //ball information : coord, speed
    PLAYER *astPlayers    //players information: coord, offset, speed, score, bonus
)
{
    /* display in-game frame */
    if( pstParams->status == -2 )
    {
        render_drawFrame( 0x0133, fPercentUpdate, pstParams, pstBall, astPlayers );
    }
    /* display special frame (message, animation, pause) */
    else
    {
        HANDLE hConsole = GetStdHandle( STD_OUTPUT_HANDLE );
        COORD coord = { 0 };
        switch( pstParams->status )
        {
            /* NEW ROUND -> animation and reset */
            case -7:
                render_drawFrame( 0x1113, fPercentUpdate, pstParams, pstBall, astPlayers );

                /* start round if animation is over */
                if( pstParams->bonus[0] >= 40 )
                {
                    pstParams->status = -2;
                    pstParams->bonus[0] = 0;
                    pstParams->clock[1] = 0;
                }
                break;

            /* NEW GAME -> instructions and wait */
            case -1:
                coord.X = 6;
                coord.Y = 38;
                SetConsoleCursorPosition( hConsole, coord );
                render_drawFrame( 0x0001, fPercentUpdate, pstParams, NULL, astPlayers );

                /* message : useable commands */
                if( pstParams->level != 4 ) //single player
                {
                    SetConsoleTextAttribute( hConsole, 0x0F );
                    printf( "  \x18\x19  " );
                    SetConsoleTextAttribute( hConsole, 0x07 );
                    printf( "D\x82placer        " );
                    SetConsoleTextAttribute( hConsole, 0x0F );
                    printf( "ESC  " );
                    SetConsoleTextAttribute( hConsole, 0x07 );
                    printf( "Afficher menu        " );
                    SetConsoleTextAttribute( hConsole, 0x0F );
                    printf( "P  " );
                    SetConsoleTextAttribute( hConsole, 0x07 );
                    printf( "Mettre en pause" );
                }
                else //2 players
                {
                    SetConsoleTextAttribute( hConsole, 0x0F );
                    printf( "ZS  " );
                    SetConsoleTextAttribute( hConsole, 0x07 );
                    printf( "D\x82placer         " );
                    SetConsoleTextAttribute( hConsole, 0x0F );
                    printf( "ESC  " );
                    SetConsoleTextAttribute( hConsole, 0x07 );
                    printf( "Menu         " );
                    SetConsoleTextAttribute( hConsole, 0x0F );
                    printf( "P  " );
                    SetConsoleTextAttribute( hConsole, 0x07 );
                    printf( "Pause         " );
                    SetConsoleTextAttribute( hConsole, 0x0F );
                    printf( "\x18\x19  " );
                    SetConsoleTextAttribute( hConsole, 0x07 );
                    printf( "D\x82placer" );
                }
                /* message : instructions */
                coord.X = 24;
                coord.Y = 19;
                SetConsoleCursorPosition( hConsole, coord );
                printf( "Appuyez sur ENTER pour commencer." );
                Sleep( 28 );
                break;

            /* PAUSE -> animation */
            case -3:
                coord.X = 35;
                coord.Y = 20;
                SetConsoleCursorPosition( hConsole, coord );
                render_drawFrame( 0x0001, 0, pstParams, pstBall, astPlayers );

                /* pause message */
                if( pstParams->clock[0]%2 == 0 )
                {
                    SetConsoleTextAttribute( hConsole, 0x0F );
                    printf( "P A U S E" );
                    SetConsoleTextAttribute( hConsole, 0x07 );
                }
                else
                {
                    printf( "P A U S E" );
                }
                Sleep( 28 );
                break;

            /* ROUND END -> message and latence */
            case -4:
            case -5:
                coord.X = 31;
                coord.Y = 19;
                SetConsoleCursorPosition( hConsole, coord );
                render_drawFrame( 0x0103, 0, pstParams, NULL, astPlayers );

                /* round message */
                if( pstParams->level != 4 )
                {
                    if( pstParams->status == -4 )
                        printf( "   Vous gagnez." );
                    else
                        printf( "   Vous perdez." );
                }
                else
                {
                    if( pstParams->status == -4 )
                        printf( "Le joueur 1 gagne." );
                    else
                        printf( "Le joueur 2 gagne." );
                }
                /* wait for one second */
                if( pstParams->clock[1] > 1 )
                {
                    pstParams->status = -6;
                }
                Sleep( 28 );
                break;

            /* GAME END -> message, latence and wait */
            case -8:
            case -9:
                render_drawFrame( 0x0301, 0, pstParams, NULL, astPlayers );

                /* message : results */
                coord.X = 27;
                if( pstParams->level != 4 ) //single player
                {
                    short sScore = 0;
                    coord.Y = 17;
                    SetConsoleCursorPosition( hConsole, coord );
                    if( pstParams->status == -8 )
                        printf( " Vous remportez la partie." );
                    else
                        printf( " Vous avec perdu la partie." );
                    /* display details : difficulty */
                    coord.X = 31;
                    coord.Y = 20;
                    SetConsoleCursorPosition( hConsole, coord );
                    printf( "Difficult\x82  : " );
                    switch( pstParams->level )
                    {
                        case 1: printf( "facile" );
                            break;
                        case 3: printf( "expert" );
                            break;
                        default: printf( "v\x82t\x82ran" );
                    }
                    /* display details : time */
                    coord.X = 31;
                    coord.Y = 21;
                    SetConsoleCursorPosition( hConsole, coord );
                    printf( "Dur\x82\x65 jeu   : %02hu:%02hu", pstParams->clock[0]/60, pstParams->clock[0]%60 );
                    /* display details : score */
                    coord.X = 31;
                    coord.Y = 22;
                    SetConsoleCursorPosition( hConsole, coord );
                    sScore = ( 2*astPlayers[0].score - astPlayers[1].score )*pstParams->level;
                    if( sScore < 0 )
                        sScore = 0;
                    printf( "Score moyen : %hd", sScore );
                    coord.Y = 25;
                }
                else //2 players
                {
                    coord.Y = 19;
                    SetConsoleCursorPosition( hConsole, coord );
                    if( pstParams->status == -8 )
                        printf( "Le joueur 1 gagne la partie." );
                    else
                        printf( "Le joueur 2 gagne la partie." );
                    /* display details : time */
                    coord.X = 31;
                    coord.Y = 21;
                    SetConsoleCursorPosition( hConsole, coord );
                    printf( "Dur\x82\x65 partie : %02hu:%02hu", pstParams->clock[0]/60, pstParams->clock[0]%60 );
                    coord.Y = 23;
                }

                /* message : instructions */
                Sleep( 800 );
                coord.X = 19;
                SetConsoleCursorPosition( hConsole, coord );
                printf( "Appuyez sur une touche pour revenir au menu." );
                /* empty buffer and wait for reaction */
                while( _kbhit() != 0 )
                {
                    _getch();
                }
                do
                {
                    _getch();
                }
                while( _kbhit() != 0 );
                pstParams->status = 1;
                render_clear();
                break;
        }//switch
    }//else
}//render_setRendering

/*-----------------------------------------------------*/

/**  Role : clear screen
  *  Output : - */
void render_clear()
{
    /* framebuffer variables */
    HWND hwnd = GetConsoleWindow();
    HDC hScreenDC = GetDC( hwnd ); //window device context
    HDC hCompatibleDC = CreateCompatibleDC( hScreenDC ); //framebuffer
    HBITMAP hbFrame = CreateCompatibleBitmap( hScreenDC, PXWIDTH, PXHEIGHT ); //bitmap

    /* set bitmap DC */
    SelectObject( hCompatibleDC, hbFrame );

    /* display full frame */
    BitBlt( hScreenDC, 0, 0, PXWIDTH, PXHEIGHT, hCompatibleDC, 0, 0, SRCCOPY );

    /* free memory and release device context */
    SelectObject( hCompatibleDC, NULL );
    DeleteObject( hbFrame );
    ReleaseDC( NULL, hScreenDC );
    DeleteDC( hCompatibleDC );
}//render_clear

/*-----------------------------------------------------*/

/**  Role : draw game frame (game state or animation)
  *  Output : - */
void render_drawFrame(
    short sModeMap,         //animation(bool), score,ball,walls (3=white, 1=grey)
    float fPercentUpdate,   //interpolation
    CONFIG *pstParams,      //parameters/bonus/timers
    BALL *pstBall,          //ball information : coord, speed
    PLAYER *astPlayers      //players information : coord, offset, speed, score, bonus
)
{
    /* variables */
    short sCoords[2] = { 0 },
          sOffsets[2] = { 0 },
          sColors[3] = { 0x0F, 0x0F, 0x0F },
          i = 0;
    /* framebuffer variables */
    HWND hwnd = GetConsoleWindow();
    HDC hScreenDC = GetDC( hwnd ); //window device context
    HDC hCompatibleDC = CreateCompatibleDC( hScreenDC ); //framebuffer
    HBITMAP hbFrame = CreateCompatibleBitmap( hScreenDC, PXWIDTH, PXHEIGHT ); //bitmap

    /* set bitmap (DC and color) */
    SelectObject( hCompatibleDC, hbFrame );
    SelectObject( hCompatibleDC, GetStockObject( NULL_PEN ) );

    /* get main colors and draw background */
    if( (sModeMap & 0x2) == 0 ) //middle line
    {
        sColors[0] = 7;
        sColors[1] = 7;
    }
    if( ((sModeMap >> 8) & 0x2) == 0 ) //scores and line
    {
        if( ((sModeMap >> 8) & 0x3) == 0 )
            sColors[2] = 0;
        else
            sColors[2] = 7;
    }
    render_drawBackground( &hCompatibleDC, sColors[0], sColors[2], astPlayers );
    #if DEBUG
        BitBlt( hCompatibleDC, 48, 25, 136, 11, hScreenDC, 48, 25, SRCCOPY );
    #endif

    /* draw bonus if not greyed */
    if( sColors[0] != 7 )
    {
        if( astPlayers[0].bonus[0] == 1 || pstParams->code >= 1 )
        {
            SelectObject( hCompatibleDC, GetStockObject( GRAY_BRUSH ) );
            Rectangle( hCompatibleDC, 11, 27, 19, 494 );
        }
        if( (sModeMap >> 12) == 0 )
        {
            render_drawBonus( &hCompatibleDC, pstParams->bonus );
        }
    }

    /* draw ball only if asked */
    if( pstBall != NULL )
    {
        /* draw ball */
        if( fPercentUpdate == 0 || (sModeMap >> 12) != 0 )
        {
            render_drawBall( &hCompatibleDC, 7+ 4*((sModeMap >> 4) & 0x2), pstBall->x, pstBall->y );
        }
        /* draw ball with interpolation */
        else
        {
            render_drawBall( &hCompatibleDC, 7 + 4*((sModeMap >> 4) & 0x2),
                             pstBall->x + fPercentUpdate*pstBall->speedX,
                            pstBall->y + fPercentUpdate*pstBall->speedY );
        }
    }

    /* draw start animation only if asked */
    if( (sModeMap >> 12) != 0 )
    {
        sOffsets[0] = astPlayers[0].offset;
        sOffsets[1] = astPlayers[1].offset;
        render_drawStartAnimation( &hCompatibleDC, pstParams->bonus, fPercentUpdate );
    }
    /* set players with bonus and interpolation */
    else
    {
        i = 0;
        do
        {
            /* define players size */
            sOffsets[i] = astPlayers[i].offset;
            switch( astPlayers[i].bonus[0] )
            {
                case 3: sOffsets[i] += 8;
                case 4:
                    /* if not greyed, display green */
                    if( sColors[i] != 7 )
                        sColors[i] = 0x0A;
                    break;
                case 6: sOffsets[i] -= 5;
                case 5:
                    /* if not greyed, display red */
                    if( sColors[i] != 7 )
                        sColors[i] = 0x0C;
                    break;
            }//switch
            /* define players position + interpolation */
            sCoords[i] = astPlayers[i].coord + fPercentUpdate*astPlayers[i].speed;
            i++;
        }
        while( i <= 1 );
    }
    /* draw players */
    render_drawPlayers( &hCompatibleDC, sCoords, sOffsets, sColors );

    /* redraw text information */
    if( pstParams->status != -2 && pstParams->status > -7 )
    {
        /* get text */
        if( pstParams->status == -3 ) //pause
        {
            BitBlt( hCompatibleDC, 280, 241, 80, 11, hScreenDC, 280, 241, SRCCOPY );
        }
        else //message
        {
            BitBlt( hCompatibleDC, 192, 229, 261, 11, hScreenDC, 192, 229, SRCCOPY );
            if( pstParams->status == -1 ) //commands
            {
                BitBlt( hCompatibleDC, 40, 456, 560, 12, hScreenDC, 40, 456, SRCCOPY );
            }
        }
    }//if

    /* display full frame */
    BitBlt( hScreenDC, 0, 0, PXWIDTH, PXHEIGHT, hCompatibleDC, 0, 0, SRCCOPY );

    /* free memory and release device context */
    SelectObject( hCompatibleDC, NULL );
    DeleteObject( hbFrame );
    ReleaseDC( NULL, hScreenDC );
    DeleteDC( hCompatibleDC );
}//render_drawFrame

/*-----------------------------------------------------*/

/**  Role : draw game background (walls, middle line, scores)
  *  Output : - */
void render_drawBackground(
    HDC *phDC,          //device context
    short sColorWalls,  //walls : hexa color code
    short sColorScores, //score numbers : hexa color code
    PLAYER *astPlayers  //players information : scores
)
{
    /* variables */
    char acScore[5] = { 0 };

    /* draw middle line and define colors */
    SelectObject( *phDC, GetStockObject( DC_BRUSH ) );
    if( sColorWalls != 7 )
    {
        SetDCBrushColor( *phDC, RGB(64,64,64) );
        Rectangle( *phDC, 318, 27, 323, 494 );
        SelectObject( *phDC, GetStockObject( WHITE_BRUSH ) );
    }
    else
    {
        SetDCBrushColor( *phDC, RGB(32,32,32) );
        Rectangle( *phDC, 318, 27, 323, 494 );
        SelectObject( *phDC, GetStockObject( GRAY_BRUSH ) );
    }

    /* draw side walls */
    Rectangle( *phDC, 2, 2, PXWIDTH-1, 19 );
    Rectangle( *phDC, 2, PXHEIGHT-18, PXWIDTH-1, PXHEIGHT-1 );

    /* set score font */
    if( sColorScores == 7 )
    {
        SetTextColor( *phDC, RGB(96,96,96) );
    }
    else if( sColorScores == 0 )
    {
        SetTextColor( *phDC, RGB(48,48,48) );
    }
    else
    {
        SetTextColor( *phDC, RGB(255,255,255) );
    }
    SetBkMode( *phDC, TRANSPARENT );
    HFONT hFont = CreateFont( 48, 0, 0, 0, 100, 0, 0, 0, ANSI_CHARSET,
                              OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY,
                              DEFAULT_PITCH|(FF_MODERN << 2), "Tahoma");

    /* draw scores text */
    SelectObject( *phDC, hFont );
    SetTextAlign( *phDC, TA_CENTER );
    sprintf( acScore, "%hd", astPlayers[0].score );
    TextOut( *phDC, CENTER_OFFSET_X - 161, 22, acScore, strlen( acScore ) );
    sprintf( acScore, "%hd", astPlayers[1].score );
    TextOut( *phDC, CENTER_OFFSET_X + 160, 22, acScore, strlen( acScore ) );
    DeleteObject( hFont );

}//render_drawBackground

/*-----------------------------------------------------*/

/**  Role : draw ball in position and with chosen color
  *  Output : - */
void render_drawBall(
    HDC *phDC,      //device context
    short sColor,   //ball hexa color code
    short sX,       //X coord
    short sY        //Y coord
)
{
    /* anti-aliasing */
    SelectObject( *phDC, GetStockObject( DC_BRUSH ) );
    if( sColor == 7 )
    {
        SetDCBrushColor( *phDC, RGB(64,64,64) );
    }
    else
    {
        SetDCBrushColor( *phDC, RGB(128,128,128) );
    }
    Rectangle( *phDC, CENTER_OFFSET_X + sX-3, CENTER_OFFSET_Y + sY-8,
               CENTER_OFFSET_X + sX+4, CENTER_OFFSET_Y + sY+9 );
    Rectangle( *phDC, CENTER_OFFSET_X + sX-5, CENTER_OFFSET_Y + sY-7,
               CENTER_OFFSET_X + sX+6, CENTER_OFFSET_Y + sY+8 );
    Rectangle( *phDC, CENTER_OFFSET_X + sX-6, CENTER_OFFSET_Y + sY-6,
               CENTER_OFFSET_X + sX+7, CENTER_OFFSET_Y + sY+7 );
    Rectangle( *phDC, CENTER_OFFSET_X + sX-8, CENTER_OFFSET_Y + sY-3,
               CENTER_OFFSET_X + sX+9, CENTER_OFFSET_Y + sY+4 );
    Rectangle( *phDC, CENTER_OFFSET_X + sX-7, CENTER_OFFSET_Y + sY-5,
               CENTER_OFFSET_X + sX+8, CENTER_OFFSET_Y + sY+6 );

    /* define color */
    if( sColor == 7 )
    {
        SelectObject( *phDC, GetStockObject( GRAY_BRUSH ) );
    }
    else
    {
        SelectObject( *phDC, GetStockObject( WHITE_BRUSH ) );
    }

    /* draw ball */
    Rectangle( *phDC, CENTER_OFFSET_X + sX-2, CENTER_OFFSET_Y + sY-8,
               CENTER_OFFSET_X + sX+3, CENTER_OFFSET_Y + sY+9 );
    Rectangle( *phDC, CENTER_OFFSET_X + sX-4, CENTER_OFFSET_Y + sY-7,
               CENTER_OFFSET_X + sX+5, CENTER_OFFSET_Y + sY+8 );
    Rectangle( *phDC, CENTER_OFFSET_X + sX-6, CENTER_OFFSET_Y + sY-5,
               CENTER_OFFSET_X + sX+7, CENTER_OFFSET_Y + sY+6 );

    Rectangle( *phDC, CENTER_OFFSET_X + sX-8, CENTER_OFFSET_Y + sY-2,
               CENTER_OFFSET_X + sX+9, CENTER_OFFSET_Y + sY+3 );
    Rectangle( *phDC, CENTER_OFFSET_X + sX-7, CENTER_OFFSET_Y + sY-4,
               CENTER_OFFSET_X + sX+8, CENTER_OFFSET_Y + sY+5 );
    Rectangle( *phDC, CENTER_OFFSET_X + sX-5, CENTER_OFFSET_Y + sY-6,
               CENTER_OFFSET_X + sX+6, CENTER_OFFSET_Y + sY+7 );
}//render_drawBall

/*-----------------------------------------------------*/

/**  Role : draw 2 players in position and with chosen color
  *  Output : - */
void render_drawPlayers(
    HDC *phDC,       //device context
    short *sCoords,  //players coords
    short *sOffsets, //players offsets (with level and bonus)
    short *sColors   //players colors (with bonus)
)
{
    /* variables */
    short i = 0,
          sXCoords[2][2] = {{ 2, 19 }, { PXWIDTH-18, PXWIDTH-1 }};

    /* draw each player */
    do
    {
        /* define drawing color */
        switch( sColors[i] )
        {
            case 7: SelectObject( *phDC, GetStockObject( GRAY_BRUSH ) );
                break;
            case 0x0A:
                SelectObject( *phDC, GetStockObject( DC_BRUSH ) );
                SetDCBrushColor( *phDC, RGB(32,192,64) );
                break;
            case 0x0C:
                SelectObject( *phDC, GetStockObject( DC_BRUSH ) );
                SetDCBrushColor( *phDC, RGB(255,80,80) );
                break;
            default: SelectObject( *phDC, GetStockObject( WHITE_BRUSH ) );
        }//switch

        /* draw rectangle */
        Rectangle( *phDC, sXCoords[i][0], CENTER_OFFSET_Y + sCoords[i] - sOffsets[i],
                   sXCoords[i][1], CENTER_OFFSET_Y + sCoords[i] + sOffsets[i] + 1 );
        i++;
    }
    while( i <= 1 );
}//render_drawPlayers

/*-----------------------------------------------------*/

/**  Role : draw available bonus with the right image
  *  Output : - */
void render_drawBonus(
    HDC *phDC,      //device context
    short *asBonus  //bonus information
)
{
    /* variable */
    short sSide = 1;

    /* display the right bonus (if any) */
    if( asBonus[0] != 0 )
    {
        /* side coord */
        if( asBonus[1] != 0 )
        {
            sSide = PXWIDTH - 22;
        }
        /* color and draw bonus */
        SelectObject( *phDC, GetStockObject( DC_BRUSH ) );
        switch( asBonus[0] )
        {
            case 1:
                SetDCBrushColor( *phDC, RGB(224,192,32) );
                RoundRect( *phDC, sSide, CENTER_OFFSET_Y + asBonus[2]-11,
                           sSide+21, CENTER_OFFSET_Y + asBonus[2]+10, 16, 16 );

                SelectObject( *phDC, GetStockObject( WHITE_BRUSH ) );
                Rectangle( *phDC, sSide + 4, CENTER_OFFSET_Y + asBonus[2]-7,
                           sSide+9, CENTER_OFFSET_Y + asBonus[2]+6 );
                Rectangle( *phDC, sSide + 12, CENTER_OFFSET_Y + asBonus[2]-7,
                           sSide+17, CENTER_OFFSET_Y + asBonus[2]+6 );
                break;
            case 2:
                SetDCBrushColor( *phDC, RGB(32,192,64) );
                RoundRect( *phDC, sSide, CENTER_OFFSET_Y + asBonus[2]-11,
                           sSide+21, CENTER_OFFSET_Y + asBonus[2]+10, 16, 16 );

                SelectObject( *phDC, GetStockObject( WHITE_BRUSH ) );
                RoundRect( *phDC, sSide+ 4, CENTER_OFFSET_Y + asBonus[2]-7,
                           sSide+17, CENTER_OFFSET_Y + asBonus[2]+6, 10, 10 );
                break;
            case 4:
                SetDCBrushColor( *phDC, RGB(32,192,64) );
                RoundRect( *phDC, sSide, CENTER_OFFSET_Y + asBonus[2]-11,
                           sSide+21, CENTER_OFFSET_Y + asBonus[2]+10, 16, 16 );

                SelectObject( *phDC, GetStockObject( WHITE_BRUSH ) );
                Rectangle( *phDC, sSide + 4, CENTER_OFFSET_Y + asBonus[2]-3,
                           sSide+17, CENTER_OFFSET_Y + asBonus[2]+2 );
                Rectangle( *phDC, sSide + 8, CENTER_OFFSET_Y + asBonus[2]-7,
                           sSide+13, CENTER_OFFSET_Y + asBonus[2]+6 );
                break;
            case 3:
                SetDCBrushColor( *phDC, RGB(32,192,64) );
                RoundRect( *phDC, sSide, CENTER_OFFSET_Y + asBonus[2]-11,
                           sSide+21, CENTER_OFFSET_Y + asBonus[2]+10, 16, 16 );

                SelectObject( *phDC, GetStockObject( WHITE_BRUSH ) );
                Rectangle( *phDC, sSide + 3, CENTER_OFFSET_Y + asBonus[2]-3,
                           sSide+8, CENTER_OFFSET_Y + asBonus[2]+6 );
                Rectangle( *phDC, sSide + 8, CENTER_OFFSET_Y + asBonus[2]-5,
                           sSide+13, CENTER_OFFSET_Y + asBonus[2]+6 );
                Rectangle( *phDC, sSide + 13, CENTER_OFFSET_Y + asBonus[2]-7,
                           sSide+18, CENTER_OFFSET_Y + asBonus[2]+6 );
                break;
            case 6:
                SetDCBrushColor( *phDC, RGB(255,80,80) );
                RoundRect( *phDC, sSide, CENTER_OFFSET_Y + asBonus[2]-11,
                           sSide+21, CENTER_OFFSET_Y + asBonus[2]+10, 16, 16 );

                SelectObject( *phDC, GetStockObject( WHITE_BRUSH ) );
                Rectangle( *phDC, sSide + 3, CENTER_OFFSET_Y + asBonus[2]-7,
                           sSide+8, CENTER_OFFSET_Y + asBonus[2]+6 );
                Rectangle( *phDC, sSide + 8, CENTER_OFFSET_Y + asBonus[2]-5,
                           sSide+13, CENTER_OFFSET_Y + asBonus[2]+6 );
                Rectangle( *phDC, sSide + 13, CENTER_OFFSET_Y + asBonus[2]-3,
                           sSide+18, CENTER_OFFSET_Y + asBonus[2]+6 );
                break;
            case 5:
                SetDCBrushColor( *phDC, RGB(255,80,80) );
                RoundRect( *phDC, sSide, CENTER_OFFSET_Y + asBonus[2]-11,
                           sSide+21, CENTER_OFFSET_Y + asBonus[2]+10, 16, 16 );

                SelectObject( *phDC, GetStockObject( WHITE_BRUSH ) );
                Rectangle( *phDC, sSide + 4, CENTER_OFFSET_Y + asBonus[2]-3,
                           sSide+17, CENTER_OFFSET_Y + asBonus[2]+2 );
                break;
            case 7:
                SetDCBrushColor( *phDC, RGB(32,80,255) );
                RoundRect( *phDC, sSide, CENTER_OFFSET_Y + asBonus[2]-11,
                           sSide+21, CENTER_OFFSET_Y + asBonus[2]+10, 16, 16 );

                SelectObject( *phDC, GetStockObject( WHITE_BRUSH ) );
                Rectangle( *phDC, sSide + 4, CENTER_OFFSET_Y + asBonus[2]-3,
                           sSide+17, CENTER_OFFSET_Y + asBonus[2]+2 );
                break;
            case 8:
                SetDCBrushColor( *phDC, RGB(32,80,255) );
                RoundRect( *phDC, sSide, CENTER_OFFSET_Y + asBonus[2]-11,
                           sSide+21, CENTER_OFFSET_Y + asBonus[2]+10, 16, 16 );

                SelectObject( *phDC, GetStockObject( WHITE_BRUSH ) );
                Rectangle( *phDC, sSide + 3, CENTER_OFFSET_Y + asBonus[2]-7,
                           sSide+8, CENTER_OFFSET_Y + asBonus[2]+6 );
                Rectangle( *phDC, sSide + 8, CENTER_OFFSET_Y + asBonus[2]-5,
                           sSide+13, CENTER_OFFSET_Y + asBonus[2]+6 );
                Rectangle( *phDC, sSide + 13, CENTER_OFFSET_Y + asBonus[2]-3,
                           sSide+18, CENTER_OFFSET_Y + asBonus[2]+6 );
                break;
        }//switch
    }//if
}//render_drawBonus

/*-----------------------------------------------------*/

/**  Role : draw round start animation
  *  Output : - */
void render_drawStartAnimation(
    HDC *phDC,      //device context
    short *asTimer, //bonus timer
    float fPercent  //interpolation
)
{
    /* variables */
    long iColor = 0;
    short sSize = 0;
    char acContent[2] = { 0 };

    /* set animation frame properties */
    acContent[0] = '3' - ( asTimer[0] - 1 )/10;
    if( acContent[0] == '0' )
    {
        sSize = 48;
        if( asTimer[0] > 35 )
        {
            iColor = 296 - 24*(( asTimer[0]-1 )%10);
        }
        else
        {
            iColor = 128 + 24*(( asTimer[0]-1 )%10);
        }
    }
    else
    {
        sSize = 48 - (( asTimer[0]-1 )%10 )*2;
        iColor = ( 12.0 - ((asTimer[0]-1)%10) )*16;
    }

    /* set font */
    SetTextAlign( *phDC, TA_LEFT );
    SetBkMode( *phDC, TRANSPARENT );
    SetTextColor( *phDC, (COLORREF)(((BYTE)iColor)|((BYTE)iColor << 8)|((BYTE)iColor << 16)) );
    HFONT hFont = CreateFont( sSize, 0, 0, 0, 100, 0, 0, 0, ANSI_CHARSET,
                              OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY,
                              DEFAULT_PITCH|(FF_MODERN << 2), "Times New Roman");
    SelectObject( *phDC, hFont );

    /* draw text */
    if( acContent[0] == '0' )
    {
        TextOut( *phDC, CENTER_OFFSET_X - 30, 216, "Go!", 3 );
    }
    else
    {
        TextOut( *phDC, CENTER_OFFSET_X + 12 - sSize/2, 240 - sSize/2, acContent, 1 );
    }
    DeleteObject( hFont );
}//render_drawStartAnimation

