/*******************************************************
 File ...... : player_ai.c
 Author .... : Romain VINDERS
 Date ...... : 19/06/2014
--------------------------------------------------------
 - set AI strategy by following action tree
 - delay reaction time
 - return more or less to center
 - follow the ball position with specific offset
 - follow the opposite of the ball position
 - roughly evaluate from far the future ball projection
 - precisely anticipate the future ball position
********************************************************/
#include <stdlib.h>
#include <math.h>
#include "../config/game_cfg.h"
#include "game_ai.h"

/*-----------------------------------------------------*/

/**  Role : run AI algorithm (set reaction delay, update strategy)
  *  Output : - */
void ai_run(
    CONFIG *pstParams,
    BALL *pstBall,
    PLAYER *astPlayers
)
{
    /* variables */
    short sEffectiveLevel = pstParams->level,
          sStrategy = 0;

    /* waiting for new action (counter to zero) */
    if( astPlayers[1].action[3] <= 0 )
    {
        /* if AI isn't trying to catch a bonus (if it is, done in function) */
        if( ai_checkBonus( pstParams, &astPlayers[1], pstBall ) == 0 )
        {
            /* if big score difference -> adjust AI level */
            if( astPlayers[1].score + 8 <= astPlayers[0].score )
            {
                sEffectiveLevel++;
            }
            else if( astPlayers[1].score - 5 >= astPlayers[0].score )
            {
                sEffectiveLevel--;
            }

            /* if ball is/was bouncing on a player -> set reaction delay */
            if( fabs(pstBall->x - pstBall->speedX) >= 290 )
            {
                ai_delay( sEffectiveLevel, &astPlayers[1], pstBall );
            }
            else
            {
                /* choose random strategy depending on level */
                switch( sEffectiveLevel )
                {
                    case 1: sStrategy = ( rand()%9 )/4;
                        break;
                    case 2: sStrategy = rand()%3;
                        break;
                    case 3: sStrategy = 1 + rand()%2;
                        break;
                    case 4: sStrategy = 2;
                        break;
                }

                /* ball approaching */
                if( pstBall->speedX > 0 )
                {
                    if( pstBall->x >= -20 ) //close distance
                    {
                        if( pstBall->x >= 130 || sStrategy == 2 )
                        {
                            ai_anticipate( sEffectiveLevel, &astPlayers[1], pstBall );
                        }
                        else
                        {
                            ai_follow( 0, sEffectiveLevel, &astPlayers[1], pstBall );
                        }
                    }
                    else //far distance
                    {
                        switch( sStrategy )
                        {
                            case 0: ai_follow( 0, sEffectiveLevel, &astPlayers[1], pstBall );
                                break;
                            case 1: ai_center( sEffectiveLevel, &astPlayers[1], pstBall );
                                break;
                            case 2: ai_farEval( &astPlayers[1], pstBall );
                                break;
                        }
                    }
                }
                /* ball away */
                else
                {
                    switch( sStrategy )
                    {
                        case 0: ai_follow( 0, sEffectiveLevel, &astPlayers[1], pstBall );
                            break;
                        case 1: ai_center( sEffectiveLevel, &astPlayers[1], pstBall );
                            break;
                        case 2:
                            if( pstBall->x >= -80 )
                                ai_follow( 1, sEffectiveLevel, &astPlayers[1], pstBall );
                            else
                                ai_farEval( &astPlayers[1], pstBall );
                            break;
                    }
                }
            }//else
        }//if
    }//if

    else
    {
        /* decrease counter */
        astPlayers[1].action[3] = astPlayers[1].action[3] - 1;

        if( astPlayers[1].action[2] != 0 ) //moving AI
        {
            /* action time ending : slow down/stop */
            if( astPlayers[1].action[3] < 3 )
            {
                if( astPlayers[1].action[1] == 0 ) //stop
                {
                    astPlayers[1].action[2] = 0;
                }
                else //slow down
                {
                    astPlayers[1].action[0] = 0;
                    if( astPlayers[1].action[1] > 0 )
                        astPlayers[1].action[1] = astPlayers[1].action[1] - 1;
                    else
                        astPlayers[1].action[1] = astPlayers[1].action[1] + 1;
                }
            }
            else
            {
                /* take care of ball speed changes */
                if( pstBall->x < -290 && pstBall->actionX < 0 && astPlayers[1].action[3] > 2 )
                {
                    astPlayers[1].action[3] = 2;
                }

                /* following ball or opposite -> update */
                if( astPlayers[1].action[2] <= 2 )
                {
                    //note : if following, action[4] is an offset, not a destination
                    if( astPlayers[1].action[2] == 2 ) //following opposite
                    {
                        sStrategy = (-1*pstBall->y/2) + astPlayers[1].action[4];
                    }
                    else //following ball
                    {
                        /* adjust offset sign */
                        if( pstBall->actionX > 0 )
                        {
                            if( (pstBall->actionY < 0) != (astPlayers[1].action[4] < 0) )
                                astPlayers[1].action[4] *= -1;
                        }
                        else if( pstBall->actionX < 0 )
                        {
                            if( (pstBall->actionY < 0) == (astPlayers[1].action[4] < 0) )
                                astPlayers[1].action[4] *= -1;
                        }
                        /* following strength depends on direction */
                        if( pstBall->actionX > 0 )
                            sStrategy = pstBall->y + astPlayers[1].action[4];
                        else
                            sStrategy = (pstBall->y/2) + astPlayers[1].action[4];
                    }

                    /* check limits */
                    if( sStrategy > 240 )
                    {
                        sStrategy = 480 - sStrategy;
                        if( sStrategy > 220 )
                            sStrategy = 220;
                    }
                    else if( sStrategy < -240 )
                    {
                        sStrategy = -480 - sStrategy;
                        if( sStrategy < -220 )
                            sStrategy = -220;
                    }

                    /* update movement */
                    if( sStrategy > astPlayers[1].coord + 8 )
                    {
                        astPlayers[1].action[0] = 1;
                        if( astPlayers[1].action[1] < 3 )
                            astPlayers[1].action[1] = astPlayers[1].action[1] + 1;
                    }
                    else if( sStrategy < astPlayers[1].coord - 8 )
                    {
                        astPlayers[1].action[0] = -1;
                        if( astPlayers[1].action[1] > -3 )
                            astPlayers[1].action[1] = astPlayers[1].action[1] - 1;
                    }
                    else
                    {
                        astPlayers[1].action[0] = 0;
                        if( astPlayers[1].action[1] > 0 )
                            astPlayers[1].action[1] = astPlayers[1].action[1] - 1;
                        else if( astPlayers[1].action[1] < 0 )
                            astPlayers[1].action[1] = astPlayers[1].action[1] + 1;
                    }
                }
                else
                {
                    /* check if bonus still exists */
                    if( astPlayers[1].action[2] == 6 && pstParams->bonus[0] == 0 )
                    {
                        astPlayers[1].action[0] = 0;
                        astPlayers[1].action[3] = 5;
                    }
                    else
                    {
                        /* destination reached : slow down/stop */
                        if( astPlayers[1].action[0] == 0 )
                        {
                            if( astPlayers[1].action[1] == 0 ) //stop
                            {
                                astPlayers[1].action[2] = 0;
                            }
                            else //slow down
                            {
                                astPlayers[1].action[1] = astPlayers[1].action[1]/2;
                            }
                        }
                        else
                        {
                            /* reaching destination */
                            if( astPlayers[1].coord + 16 >= astPlayers[1].action[4]
                             && astPlayers[1].coord - 16 <= astPlayers[1].action[4] )
                            {
                                astPlayers[1].action[0] = 0;
                            }
                        }
                    }//else
                }//else (not following)
            }//else (time remaining)
        }//if (moving)
    }//else (action)
}//ai_run

/*-----------------------------------------------------*/

/**  Role : reaction delay
  *  Output : - */
void ai_delay(
    short sDifficulty,  //AI intelligence level (0 to 4)
    PLAYER *pstPlayer,  //AI player information
    BALL *pstBall       //ball information
)
{
    short sTmp = 0;

    /* no action for a variable time */
    pstPlayer->action[0] = 0;
    pstPlayer->action[2] = 0;
    if( pstBall->actionX < 0 ) //ball away -> slower
    {
        /* low difficulty -> random immobility */
        if( sDifficulty <= 1 && rand()%3 == 0 )
        {
            /* set time cycles */
            if( pstBall->actionX != 0 )
                sTmp = (pstBall->x + 280)/fabs(pstBall->actionX); //cycles before reaching P1
            if( sTmp > 10 )
                pstPlayer->action[3] = 10 + rand()%(sTmp - 10);
            else
                pstPlayer->action[3] = sTmp;
        }
        else
        {
            pstPlayer->action[3] = 4 + (3 - sDifficulty)*2 + rand()%3;//(100->600ms)/50
        }
    }
    else //ball approaching -> faster
    {
        pstPlayer->action[3] = 3 + (3 - sDifficulty) + rand()%2;//(100->350ms)/50
    }
}//ai_delay

/*-----------------------------------------------------*/

/**  Role : go near the center area
  *  Output : - */
void ai_center(
    short sDifficulty,  //AI intelligence level (0 to 4)
    PLAYER *pstPlayer,  //AI player information
    BALL *pstBall       //ball information
)
{
    /* set action (center) */
    pstPlayer->action[2] = 3;
    if( pstBall->actionX > 0 ) //ball approaching
    {
        /* set time cycles */
        pstPlayer->action[3] = -2 + rand()%(5 - sDifficulty) + (-1*pstBall->x/pstBall->actionX);
        /* set destination */
        if( pstBall->y > 32 )
        {
            pstPlayer->action[4] = -20 - rand()%16;
        }
        else if( pstBall->y < 32 )
        {
            pstPlayer->action[4] = 20 + rand()%16;
        }
        else
        {
            pstPlayer->action[4] = 0;
        }
    }
    else if( pstBall->actionX < 0 ) //ball away
    {
        /* set time cycles + destination */
        pstPlayer->action[4] = 0;
        if( sDifficulty <= 1 || (sDifficulty == 2 && rand()%2 == 0) )
        {
            pstPlayer->action[3] = (pstBall->x + 294)/fabs(pstBall->actionX); //cycles
            /* destination */
            if( pstBall->y > 32 )
            {
                pstPlayer->action[4] = 20 + rand()%16;
            }
            else if( pstBall->y < 32 )
            {
                pstPlayer->action[4] = -20 - rand()%16;
            }
        }
        else
        {
            pstPlayer->action[3] = (pstBall->x + 94 + rand()%200)/fabs(pstBall->actionX); //cycles
            /* destination */
            if( pstBall->y > 32 )
            {
                pstPlayer->action[4] = -20 - rand()%16;
            }
            else if( pstBall->y < 32 )
            {
                pstPlayer->action[4] = 20 + rand()%16;
            }
        }
    }
    if( pstPlayer->action[3] > 0 )
    {
        /* set speed */
        pstPlayer->action[1] = 1 + fabs( (pstPlayer->coord - pstPlayer->action[4])/pstPlayer->action[3] );
        if( pstPlayer->action[1] > 3 )
            pstPlayer->action[1] = 3;

        /* set direction */
        if( pstPlayer->action[4] < pstPlayer->coord - 8 )
        {
            pstPlayer->action[1] *= -1;
            pstPlayer->action[0] = -1;
        }
        else if( pstPlayer->action[4] > pstPlayer->coord + 8 )
        {
            pstPlayer->action[0] = 1;
        }
        else
        {
            pstPlayer->action[0] = 0;
            pstPlayer->action[1] = 0;
        }
    }
}//ai_center

/*-----------------------------------------------------*/

/**  Role : follow with offset the ball Y coord
  *  Output : - */
void ai_follow(
    short sMode,        //follow mode (0=straight, 1=opposite)
    short sDifficulty,  //AI intelligence level (0 to 4)
    PLAYER *pstPlayer,  //AI player information
    BALL *pstBall       //ball information
)
{
    /* variables */
    short sDestination = 0;

    /* set destination, offset, cycles */
    if( sMode == 0 ) //straight
    {
        pstPlayer->action[2] = 1;
        if( pstBall->actionX > 0 )
        {
            pstPlayer->action[4] = (1 + (sDifficulty*48/pstBall->actionX))*pstBall->actionY + (-4 + rand()%8);
            if( pstBall->x < -20 )
            {
                pstPlayer->action[4] <<= 1;
                pstPlayer->action[3] = (-16 + (rand()%16) - pstBall->x)/pstBall->actionX;
            }
            else
            {
                pstPlayer->action[3] = fabs(150 - sDifficulty*2 - pstBall->x)/pstBall->actionX;
            }
            sDestination = pstBall->y + pstPlayer->action[4];
        }
        else
        {
            pstPlayer->action[4] = -1*pstBall->actionY + (-16 + rand()%32);
            sDestination = pstBall->y/2 + pstPlayer->action[4];
            pstPlayer->action[3] = (-294 - pstBall->x)/pstBall->actionX;
        }
    }
    else //opposite
    {
        /* destination and offset */
        pstPlayer->action[2] = 2;
        pstPlayer->action[4] = -8 + rand()%16;
        sDestination = pstBall->y/2 + pstPlayer->action[4];

        /* time cycles */
        if( pstBall->actionX > 0 )
            pstPlayer->action[3] = (150 - pstBall->x)/pstBall->actionX;
        else
            pstPlayer->action[3] = (-294 - pstBall->x)/pstBall->actionX;
    }

    /* check limits */
    if( sDestination > 240 )
    {
        sDestination = 480 - sDestination;
        if( sDestination > 220 )
            sDestination = 220;
    }
    else if( sDestination < -240 )
    {
        sDestination = -480 - sDestination;
        if( sDestination < -220 )
            sDestination = -220;
    }

    /* set speed and direction */
    if( sDestination > pstPlayer->coord + 8 )
    {
        pstPlayer->action[0] = 1;
        if( pstBall->x >= -20 || fabs(sDestination - pstPlayer->coord) > 100 )
            pstPlayer->action[1] = 3;
        else
            pstPlayer->action[1] = 2;
    }
    else if( sDestination < pstPlayer->coord - 8 )
    {
        pstPlayer->action[0] = -1;
        if( pstBall->x >= -20 || fabs(sDestination - pstPlayer->coord) > 100 )
            pstPlayer->action[1] = -3;
        else
            pstPlayer->action[1] = -2;
    }
    else
    {
        pstPlayer->action[0] = 0;
        pstPlayer->action[1] = 0;
    }
}//ai_follow

/*-----------------------------------------------------*/

/**  Role : evaluate approximation based on ball angle
  *  Output : - */
void ai_farEval(
    PLAYER *pstPlayer,  //AI player information
    BALL *pstBall       //ball information
)
{
    /* variables */
    short sCycles = 0,
          sProjection = 0;

    if( pstBall->actionX != 0 )
    {
        /* set action (projection) */
        pstPlayer->action[2] = 4;

        /* find destination time */
        if( pstBall->actionX > 0 )
            sCycles = (295 - pstBall->x)/pstBall->actionX;
        else
            sCycles = (895 + pstBall->x)/fabs(pstBall->actionX);

        /* set destination projection */
        sProjection = pstBall->y + sCycles*pstBall->actionY;
        if( fabs(sProjection) > 240 && fabs(sProjection) <= 720 ) //bouncing
        {
            /* find real projection */
            if( sProjection > 0 )
                sProjection = 480 - sProjection;
            else
                sProjection = -480 - sProjection;

            /* between projection and ball position */
            if( pstBall->actionX > 0 )
                pstPlayer->action[4] = (2*sProjection + pstBall->y)/4;
            else
                pstPlayer->action[4] = (sProjection + pstBall->y)/2;

            /* randomness *-/
            pstPlayer->action[4] = ( pstPlayer->action[4]
                                    *(100 + (-8 + rand()%16)) )/100; //randomness*/
        }
        else //no bouncing or too many -> between projection and ball opposite
        {
            if( sProjection > 480) //2+ bounces
            {
                if( sProjection > 600 )
                    sProjection = -1*pstBall->y/4;
                else
                    sProjection = pstBall->y/4;
            }
            if( pstBall->actionX > 0 )
            {
                pstPlayer->action[4] = (2*sProjection - pstBall->y)/3;
                /*pstPlayer->action[4] = ( pstPlayer->action[4]
                                        *(100 + (-4 + rand()%8)) )/100; //randomness*/
            }
            else
            {
                pstPlayer->action[4] = (sProjection - 2*pstBall->y)/4;
                /*pstPlayer->action[4] = ( pstPlayer->action[4]
                                        *(100 + (-8 + rand()%16)) )/100; //randomness*/
            }
        }
        if( fabs(pstPlayer->action[4]) > 180 )
            pstPlayer->action[4] = 3*pstPlayer->action[4]/4;

        /* set time cycles */
        if( pstBall->actionX > 0 )
            pstPlayer->action[3] = -1*pstBall->x/pstBall->actionX;
        else
            pstPlayer->action[3] = (295 + pstBall->x)/fabs(pstBall->actionX);

        /* set direction and speed */
        if( pstPlayer->action[4] > pstPlayer->coord + 32 )
        {
            pstPlayer->action[0] = 1;
            if( fabs(pstPlayer->action[4] - pstPlayer->coord) > 100 )
                pstPlayer->action[1] = 3;
            else
                pstPlayer->action[1] = 2;
        }
        else if( pstPlayer->action[4] < pstPlayer->coord - 32 )
        {
            pstPlayer->action[0] = -1;
            if( fabs(pstPlayer->action[4] - pstPlayer->coord) > 100 )
                pstPlayer->action[1] = -3;
            else
                pstPlayer->action[1] = -2;
        }
        else
        {
            pstPlayer->action[0] = 0;
            pstPlayer->action[1] = 0;
        }
    }//if
}//ai_farEval

/*-----------------------------------------------------*/

/**  Role : evaluate future ball position and try to get there
  *  Output : - */
void ai_anticipate(
    short sDifficulty,  //AI intelligence level (0 to 4)
    PLAYER *pstPlayer,  //AI player information
    BALL *pstBall       //ball information
)
{
    /* variables */
    short sErreur = 0,
          sDestination = 0;
    float fBallOffset = (290.0 - pstBall->x)*(float)pstBall->actionY/(float)pstBall->actionX;

    if( pstBall->actionX > 0 )
    {
        /* set action and time cycles */
        pstPlayer->action[2] = 5;
        if( pstBall->x >= 200 )
        {
            pstPlayer->action[3] = 1 + rand()%4 + (310 - pstBall->x)/pstBall->actionX;
        }
        else if( pstBall->x >= 160 )
        {
            pstPlayer->action[3] = 1 + (200 - pstBall->x)/pstBall->actionX;
            sErreur = ( rand()%( pstBall->actionX + 20 ) )/8;
        }
        else if( pstBall->x >= 130 )
        {
            pstPlayer->action[3] = 1 + (170 - pstBall->x)/pstBall->actionX;
            sErreur = ( rand()%( pstBall->actionX + (short)fabs(294 - pstBall->x) ) )/16;
        }
        else
        {
            pstPlayer->action[3] = 1 + (140 - pstBall->x)/pstBall->actionX;
            sErreur = ( rand()%( pstBall->actionX + (short)fabs(294 - pstBall->x) ) )/8;
        }

        /* set destination */
        sDestination = pstBall->y + (short)fBallOffset;
        if( sDestination > 240 )
        {
            sDestination = 480 - sDestination;
            if( sDestination < -240 )
            {
                sDestination = -470 - sDestination;
                if( sDestination > 240 )
                    sDestination = 220;
            }
        }
        else if( sDestination < -240 )
        {
            sDestination = -480 - sDestination;
            if( sDestination > 240 )
            {
                sDestination = 470 - sDestination;
                if( sDestination < -240 )
                    sDestination = -220;
            }
        }
        if( rand()%2 == 0 )
            pstPlayer->action[4] = sDestination + sErreur;
        else
            pstPlayer->action[4] = sDestination - sErreur;

        /* set speed and direction */
        if( pstPlayer->action[4] > pstPlayer->coord + 2 )
        {
            pstPlayer->action[0] = 1;
            pstPlayer->action[1] = 3;
        }
        else if( pstPlayer->action[4] < pstPlayer->coord - 2 )
        {
            pstPlayer->action[0] = -1;
            pstPlayer->action[1] = -3;
        }
    }//if
}//ai_anticipate

/*-----------------------------------------------------*/

/**  Role : check if bonus, evaluate if enough time, adjust action
  *  Output : decision (0=cancel, 1=get bonus) */
short ai_checkBonus(
    CONFIG *pstParams,  //parameters : level, bonus information
    PLAYER *pstPlayer,  //AI player information
    BALL *pstBall       //ball information
)
{
    /* variables */
    short sStatus = 0,
          sBallCycles = 0,
          sDistanceCycles = 0;

    /* check bonus */
    if( pstParams->bonus[0] != 0 && pstParams->bonus[0] <= 4 && pstParams->bonus[1] == 1 )
    {
        /* find time before ball arrives */
        if( pstBall->actionX > 0 )
        {
            sBallCycles = (294 - pstBall->x)/pstBall->actionX;
        }
        else if( pstBall->actionX < 0 )
        {
            sBallCycles = 588 + ( (-294 - pstBall->x)/pstBall->actionX );
        }

        /* find time to catch bonus */
        sDistanceCycles = fabs(pstPlayer->coord - pstParams->bonus[2])
                          /( 3*((PLAYER_SPEED/2) + pstParams->level) );

        /* check time to catch and come back (error depending on speed and level) */
        if( sBallCycles - rand()%(1 + (short)fabs(pstBall->actionX)/16) >= 2*sDistanceCycles - rand()%(5 - pstParams->level) )
        {
            /* set time cycles and destination */
            sStatus = 1;
            pstPlayer->action[2] = 6;
            pstPlayer->action[3] = sDistanceCycles;

            /* set speed and direction */
            if( pstParams->bonus[2] > pstPlayer->coord )
            {
                pstPlayer->action[0] = 1;
                pstPlayer->action[1] = 3;
                pstPlayer->action[4] = pstParams->bonus[2] - 20;
            }
            else if( pstParams->bonus[2] < pstPlayer->coord )
            {
                pstPlayer->action[0] = -1;
                pstPlayer->action[1] = -3;
                pstPlayer->action[4] = pstParams->bonus[2] + 20;
            }
        }
    }
    return sStatus;
}//ai_checkBonus
