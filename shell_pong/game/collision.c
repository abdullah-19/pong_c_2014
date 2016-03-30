/*******************************************************
 File ...... : collision.c
 Author .... : Romain VINDERS
 Date ...... : 17/06/2014
--------------------------------------------------------
 - initialize ball speed and position
 - move players as asked (limited by walls and ball)
 - move ball as asked (limited by players and walls)
 - calculate collisions between ball and walls/players
********************************************************/
#include <stdlib.h>
#include <math.h>
#include "collision.h"

#define PI 3.14159265
/*-----------------------------------------------------*/

/**  Role : set initial ball position and speed
  *  Output : - */
void game_createBall(
    short sLevel,   //difficulty level (or 2 players)
    BALL *pstBall   //ball information
)
{
    /* variables */
    short sAbsoluteSpeed = 0;

    /* set absolute ball speed */
    if( sLevel != 4 )
    {
        sAbsoluteSpeed = BALL_START_SPEED - (3 - sLevel);
    }
    else
    {
        sAbsoluteSpeed = BALL_START_SPEED - 2;
    }

    /* set relative speeds (X and Y) */
    /* sX = speed * (-1 or 1) * (0.375 to 0.75) */
    pstBall->actionX = 1 + ( (1 | (rand()%2 - 1)) * (4 + rand()%4) * sAbsoluteSpeed )/8;
    /* sY = (-1 or 1) * sqrt( speed^2 - sX^2 ) */
    pstBall->actionY = (1 | (rand()%2 - 1)) * sqrt( sAbsoluteSpeed*sAbsoluteSpeed
                                                  - pstBall->actionX*pstBall->actionX );

    /* set ball coords */
    pstBall->speedX = pstBall->actionX;
    pstBall->speedY = pstBall->actionY;
    pstBall->x = pstBall->actionX*(-3.4 + rand()%2); //opposite way & random factor
    pstBall->y = pstBall->actionY*(1 - rand()%5); //random
}//createBall

/*-----------------------------------------------------*/

/**  Role : move players and check collisions
  *  Output : - */
void game_movePlayers(
    CONFIG *pstParams,  //parameters : difficulty
    BALL *pstBall,      //ball information : coord
    PLAYER *astPlayers  //player information : coord, speed, offset, action, bonus
)
{
    /* variables */
    short sOffset = 0,
          sMaxCoord = 0,
          i = 0;

    /* update players */
    do
    {
        sOffset = astPlayers[i].offset;
        /* if IA, adjust speed to difficulty */
        if( pstParams->level != 4 )
        {
            if( i == 0 )
            {
                astPlayers[i].speed = ( astPlayers[i].action[1]
                                        * (PLAYER_SPEED - 2*pstParams->level) )/3;
            }
            else
            {
                astPlayers[i].speed = ( astPlayers[i].action[1]
                                        * ((PLAYER_SPEED/2) + pstParams->level) )/3;
            }
        }
        else
        {
            astPlayers[i].speed = ( astPlayers[i].action[1]
                                    * (PLAYER_SPEED - 4) )/3;
        }

        /* apply bonus */
        if( astPlayers[i].bonus[0] > 2 )
        {
            switch( astPlayers[i].bonus[0] )
            {
                case 3: sOffset += 8; //bigger
                    break;
                case 6: sOffset -= 5; //smaller
                    break;
                case 4: astPlayers[i].speed *= 1.4; //faster
                    break;
                case 5: astPlayers[i].speed *= 0.6; //slower
                    break;
            }//switch
        }

        /* check if ball is in side contact */
        if( (i == 0 && pstBall->x < -302) || (i == 1 && pstBall->x > 302) )
        {
            if( astPlayers[i].coord + sOffset >= pstBall->y - 9
             && pstBall->y - 8 >= astPlayers[i].coord + sOffset - fabs(pstBall->actionY) )
            {
                if( astPlayers[i].speed > 0 )
                {
                    sOffset = -1;
                    astPlayers[i].speed = 0;
                }
                pstBall->timeout++;
                //if( pstBall->timeout == 1 && pstBall->actionY < 0 && fabs(pstBall->x) > 310 )
                    //pstBall->actionY *= -1;
            }
            else if( astPlayers[i].coord - sOffset <= pstBall->y + 9
                  && pstBall->y + 8 <= astPlayers[i].coord + fabs(pstBall->actionY) - sOffset )
            {
                if( astPlayers[i].speed < 0 )
                {
                    sOffset = -1;
                    astPlayers[i].speed = 0;
                }
                pstBall->timeout++;
                //if( pstBall->timeout == 1 && pstBall->actionY > 0 && fabs(pstBall->x) > 310 )
                    //pstBall->actionY *= -1;
            }
        }

        /* move player and check max */
        if( sOffset != -1 )
        {
            astPlayers[i].coord += astPlayers[i].speed;
            sMaxCoord = fabs( astPlayers[i].coord ) + sOffset;
            if( sMaxCoord > MAX_COORD ) //player at max position -> stop
            {
                astPlayers[i].speed = 0;
                if( astPlayers[i].coord > 0 )
                    astPlayers[i].coord = MAX_COORD - sOffset;
                else
                    astPlayers[i].coord = MIN_COORD + sOffset;
                if( i == 0 || pstParams->level == 4 ) //reset action if not AI
                {
                    astPlayers[i].action[0] = 0;
                    astPlayers[i].action[1] = 0;
                }
            }
            /* slow interpolation if near max position */
            else if( sMaxCoord + fabs( astPlayers[i].speed ) > MAX_COORD )
            {
                if( astPlayers[i].speed > 0 )
                    astPlayers[i].speed = MAX_COORD - sMaxCoord;
                else
                    astPlayers[i].speed = sMaxCoord - MAX_COORD;
            }
            /* slow interpolation if player stops */
            else if( astPlayers[i].action[0] == 0 && astPlayers[i].speed != 0 )
            {
                if( fabs( astPlayers[i].action[1] ) == 2 )
                {
                    astPlayers[i].speed >>= 1;
                    astPlayers[i].coord += astPlayers[i].speed;
                }
                else
                {
                    astPlayers[i].speed = 0;
                }
            }
        }
        i++;
    }
    while( i <= 1 );
}//game_movePlayers

/*-----------------------------------------------------*/

/**  Role : move ball and check collisions/victory
  *  Output : indicator (1=collision, 2=victory (P1), 3=victory (P2)) */
short game_moveBall(
    CONFIG *pstParams,  //parameters
    BALL *pstBall,      //ball information : coord and speed
    PLAYER *astPlayers  //players informations : coord, offset, bonus
)
{
    /* variables */
    short sIndicator = 0,
          sTmp = 0;
    float fSpeed = 0;

    /* move ball theorically */
    pstBall->x += pstBall->speedX;
    pstBall->y += pstBall->speedY;

  /* collisions with players
     ----------------------- */
    if( pstBall->x >= 294 && pstBall->speedX > 0 ) //right player
    {
        /* right goal (ball out of screen) */
        if( pstBall->x >= 328 && pstBall->x - pstBall->speedX >= 290 )
        {
            sIndicator = 2;
        }
        /* check collision */
        else
        {
            if( game_setBallCollision( pstBall, &astPlayers[1], &fSpeed ) != 0 )//collision
            {
                sIndicator = 1;
                pstBall->speedX = pstBall->actionX;
            }
        }
    }
    else if( pstBall->x <= -294 && pstBall->speedX < 0 ) //left player
    {
        /* god bonus or cheat code */
        if( astPlayers[0].bonus[0] == 1 || pstParams->code == 1 )
        {
            pstBall->actionX *= -1;
            pstBall->speedX = pstBall->actionX;
            sIndicator = 1;
        }
        /* left goal (ball out of screen) */
        else if( pstBall->x <= -328 && pstBall->x - pstBall->speedX <= -290 )
        {
            sIndicator = 3;
        }
        /* check collision */
        else
        {
            if( game_setBallCollision( pstBall, &astPlayers[0], &fSpeed ) != 0 )//collision
            {
                sIndicator = 1;
                pstBall->speedX = pstBall->actionX;
            }
        }
    }
    /* prepare next collision (to avoid collision miss, and for interpolation) */
    else
    {
        if( pstBall->speedX < 0 )
        {
            if( pstBall->x < 0 && astPlayers[0].bonus[0] != 1 && pstParams->code != 1 )
                game_prepareCollision( pstBall, &astPlayers[0] );
        }
        else
        {
            if( pstBall->x > 0 )
                game_prepareCollision( pstBall, &astPlayers[1] );
        }
    }


  /* collisions with walls
     --------------------- */
    if( fabs(pstBall->y) >= 234 )
    {
        sIndicator = 1;
        pstBall->actionY *= -1;

        /* if too vertical -> speed up X axis */
        if( 2*fabs(pstBall->actionX) < fabs(pstBall->actionY) )
        {
            if( pstBall->actionX == 0 ) //vertical
                pstBall->actionX = ((1 - rand()%2 )^1)*8;
            else //oblique
                pstBall->actionX = (float)pstBall->actionX*(17.0 + rand()%3)/16.0;
            pstBall->speedX = pstBall->actionX;
        }
        else
        {
            /* if perfect diagonal (corner to corner or half) -> change */
            sTmp = (fabs(pstBall->actionX)*0.8125) - fabs(pstBall->actionY);
            if( sTmp > -1 && sTmp < 1 )
            {
                pstBall->actionY = (float)pstBall->actionY*1.25;
            }
            sTmp = (fabs(pstBall->actionX)*0.40625) - fabs(pstBall->actionY);
            if( sTmp > -2 && sTmp < 2 )
            {
                pstBall->actionX = (float)pstBall->actionX*1.2;
                pstBall->speedX = pstBall->actionX;
            }
            /* if not max speed yet -> speed up */
            if( fabs(pstBall->actionX) < (43 + pstParams->level) )
            {
                pstBall->actionX *= (10.0 + (float)( rand()%(5 + (pstParams->level/2)) )/2.0)/10.0;
                pstBall->speedX = pstBall->actionX;
            }
            else //random variations
            {
                sTmp = rand()%3;
                if( sTmp == 1 )
                {
                    pstBall->actionY = (float)pstBall->actionY*0.9;
                    pstBall->actionX = (float)pstBall->actionX*1.1;
                    pstBall->speedX = pstBall->actionX;
                }
                else if( sTmp == 2 )
                {
                    pstBall->actionY = (float)pstBall->actionY*1.1;
                    pstBall->actionX = (float)pstBall->actionX*0.9;
                    pstBall->speedX = pstBall->actionX;
                }
            }
        }
        pstBall->speedY = pstBall->actionY;

        /* reposition if too far */
        if( pstBall->y > 234 )
            pstBall->y = 234;
        else if( pstBall->y < -234 )
            pstBall->y = -234;
    }
    else
    {
        /* check if future collision next time -> slow down interpolation */
        sTmp = fabs(pstBall->y + pstBall->actionY);
        if( sTmp > 234 )
        {
            sTmp -= 234;
            if( pstBall->speedY > 0 )
            {
                pstBall->speedY -= sTmp;
            }
            else
            {
                pstBall->speedY += sTmp;
            }
            pstBall->speedX *= (pstBall->speedY/pstBall->actionY);
        }
    }

    /* set timeout to avoid fatal errors */
    if( sIndicator == 1 )
    {
        pstBall->timeout++;
    }
    else if( pstBall->timeout > 0 )
    {
        pstBall->timeout--;
    }

    /* prevent freeze bug */
    if( pstBall->speedX == 0 )
    {
        pstBall->speedX = pstBall->actionX;
    }
    return sIndicator;
}//game_moveBall

/*-----------------------------------------------------*/

/**  Role : set ball collision on a player
  *  Output : collision (0=no, 1=yes) */
short game_setBallCollision(
    BALL *pstBall,      //ball information
    PLAYER *pstPlayer,  //player information
    float *pfSpeed      //linear speed
)
{
    /* variables */
    short sCollision = 1,
          sOffset = pstPlayer->offset,
          sInX = fabs(pstBall->x) - 294,
          sInY = 0;

    /* get player offset (bonus) and find Y contact */
    if( pstPlayer->bonus[0] > 2 )
    {
        switch( pstPlayer->bonus[0] )
        {
            case 3: sOffset += 8; //bigger
                break;
            case 6: sOffset -= 5; //smaller
                break;
        }
    }
    if( pstPlayer->coord > pstBall->y ) //top contact
        sInY = pstBall->y - ( pstPlayer->coord - (sOffset + 8) );
    else //bottom contact
        sInY = pstPlayer->coord + sOffset + 8 - pstBall->y;

    /* no collision */
    if( sInY < 0 || ( sInY == 0 && sInX < 2 ) )
    {
        sCollision = 0;
    }

    /* collision with player */
    else
    {
      /* frontal collision
         ----------------- */
        if( pstBall->timeout == 0 &&
           (( pstBall->speedY >= 0
              && pstBall->y <= pstPlayer->coord + sOffset + 6
              && pstBall->y >= pstPlayer->coord + sInX*pstBall->speedY/16 - (sOffset + 6) )
         || ( pstBall->speedY < 0
              && pstBall->y <= pstPlayer->coord + sInX*pstBall->speedY/16 + sOffset + 6
              && pstBall->y >= pstPlayer->coord - (sOffset + 6) )) )
        {
            pstBall->actionX *= -1;
            /* if not at max coord */
            if( fabs(pstPlayer->coord) + sOffset + 8 <= MAX_COORD )
            {
                /* follow player movement */
                if( pstPlayer->speed > 0 )
                    pstBall->actionY += 2;
                else if( pstPlayer->speed < 0 )
                    pstBall->actionY -= 2;
                /* reposition */
                if( pstBall->x > 294 )
                {
                    pstBall->x = 294;
                }
                else if( pstBall->x < -294 )
                {
                    pstBall->x = -294;
                }
            }
            else //max coord -> offset to avoid collision loop
            {
                /* reposition */
                if( pstBall->x >= 294 )
                {
                    pstBall->x = 293;
                }
                else if( pstBall->x <= -294 )
                {
                    pstBall->x = -293;
                }
            }

            /* ajust angle if too horizontal */
            if( fabs(pstBall->actionX) > 2*fabs(pstBall->actionY) )
            {
                if( pstBall->actionY == 0 )
                    pstBall->actionY = ((1 - rand()%2)^1)*8;
                else
                    pstBall->actionY = (float)pstBall->actionY*(17.0 + rand()%3)/16.0;
            }
            /* ajust angle if too vertical */
            else if( fabs(pstBall->actionY) > 2*fabs(pstBall->actionX) )
            {
                pstBall->actionX = (float)pstBall->actionX*(17.0 + rand()%3)/16.0;
                pstBall->speedX = pstBall->actionX;
            }
            pstBall->speedY = pstBall->actionY;
        }

      /* side collision
         ----------------- */
        else
        {
            /* double collision wall/player (corner) -> invert X speed */
            if( ( pstPlayer->coord - (sOffset + 8) <= MIN_COORD && pstBall->y - sOffset <= MIN_COORD )
             || ( pstPlayer->coord + sOffset + 8 >= MAX_COORD && pstBall->y + sOffset >= MAX_COORD ) )
            {
                if( ( pstBall->x > 0 && pstBall->actionX > 0 )
                 || ( pstBall->x < 0 && pstBall->actionX < 0 ) )
                    pstBall->actionX *= -1;
            }
            else
            {
                /* invert Y speed */
                pstBall->actionY *= -1;
                pstBall->speedY = pstBall->actionY;

                /* reposition */
                sInY++;
                if( sInY > 0 )
                {
                    if( pstPlayer->coord < pstBall->y )
                        sInY *= -1;
                    pstBall->y += sInY;
                }
            }
        }
    }
    return sCollision;
}//game_setBallCollision

/*-----------------------------------------------------*/

/**  Role : prepare ball collision on a player the next time
  *  Output : -- */
void game_prepareCollision(
    BALL *pstBall,      //ball information
    PLAYER *pstPlayer   //player information
)
{
    /* variables */
    short sNextX = pstBall->x + pstBall->actionX,
          sNextY = pstBall->y + pstBall->actionY,
          sInX = fabs(sNextX) - 294,
          sInY = 0,
          sNextPlayer = pstPlayer->coord + pstPlayer->speed,
          sOffset = pstPlayer->offset;

    /* set default speed */
    pstBall->speedX = pstBall->actionX;

    /* check future collision and adapt speed */
    if( sInX > 0 )
    {
        /* get player offset (bonus) and find Y contact */
        if( pstPlayer->bonus[1] > 1 )
        {
            switch( pstPlayer->bonus[0] )
            {
                case 3: sOffset += 8; //bigger
                    break;
                case 6: sOffset -= 5; //smaller
                    break;
            }
        }
        if( sNextPlayer > sNextY ) //top contact
            sInY = sNextY - ( sNextPlayer - (sOffset + 8) );
        else //bottom contact
            sInY = sNextPlayer + sOffset + 8 - sNextY;

        /* future collision -> adapt speed to be at the right pixel */
        if( sInY > 0 || ( sInY == 0 && sInX >= 2 ) )
        {
            /* front collision */
            if( ( pstBall->actionY >= 0
                  && sNextY <= sNextPlayer + sOffset + 6
                  && sNextY >= sNextPlayer + sInX*pstBall->speedY/12 - (sOffset + 6) )
             || ( pstBall->actionY < 0
                  && sNextY <= sNextPlayer + sInX*pstBall->speedY/12 + sOffset + 6
                  && sNextY >= sNextPlayer - (sOffset + 6) ) )
            {
                /* adapt max X speed to limit */
                if( sInX > fabs(pstBall->speedX)/2 )
                {
                    sInX /= 2;
                }
                if( pstBall->actionX >= 0 )
                {
                    pstBall->speedX -= sInX;
                    if( pstBall->speedX <= 0 )
                        pstBall->speedX = 1;
                }
                else
                {
                    pstBall->speedX += sInX;
                    if( pstBall->speedX >= 0 )
                        pstBall->speedX = -1;
                }
                /* adapt Y speed proportionally */
                pstBall->speedY = (float)pstBall->actionY * ( (float)pstBall->speedX/(float)pstBall->actionX );
            }
            else
            {
                /* take the smallest overflow -> move it to 0 and adapt the other speed */
                if( sInX >= sInY )
                {
                    if( pstBall->actionY >= 0 )
                    {
                        pstBall->speedY = pstBall->actionY - sInY;
                        if( pstBall->speedY <= 0 )
                            pstBall->speedY = 1;
                    }
                    else
                    {
                        pstBall->speedY = pstBall->actionY + sInY;
                        if( pstBall->speedY >= 0 )
                            pstBall->speedY = -1;
                    }
                    pstBall->speedX = (float)pstBall->actionX * ( (float)pstBall->speedY/(float)pstBall->actionY );
                }
                else
                {
                    if( pstBall->actionX >= 0 )
                    {
                        pstBall->speedX -= sInX;
                        if( pstBall->speedX <= 0 )
                            pstBall->speedX = 1;
                    }
                    else
                    {
                        pstBall->speedX += sInX;
                        if( pstBall->speedX >= 0 )
                            pstBall->speedX = -1;
                    }
                    pstBall->speedY = (float)pstBall->actionY * ( (float)pstBall->speedX/(float)pstBall->actionY );
                }
            }
        }
    }//if
}//game_prepareCollision
