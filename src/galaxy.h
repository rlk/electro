/*    Copyright (C) 2005 Robert Kooima                                       */
/*                                                                           */
/*    ELECTRO is free software;  you can redistribute it and/or modify it    */
/*    under the terms of the  GNU General Public License  as published by    */
/*    the  Free Software Foundation;  either version 2 of the License, or    */
/*    (at your option) any later version.                                    */
/*                                                                           */
/*    This program is distributed in the hope that it will be useful, but    */
/*    WITHOUT  ANY  WARRANTY;  without   even  the  implied  warranty  of    */
/*    MERCHANTABILITY or  FITNESS FOR A PARTICULAR PURPOSE.   See the GNU    */
/*    General Public License for more details.                               */

#ifndef GALAXY_H
#define GALAXY_H

#include "node.h"
#include "star.h"

/*---------------------------------------------------------------------------*/

struct galaxy
{
    int    S_num;
    int    N_num;

    float  magn;
    float  bound[6];

    struct star *S;
    struct node *N;
};

/*---------------------------------------------------------------------------*/

int  galaxy_init(void);
void galaxy_draw(int, int, const float[16]);

void galaxy_prep(void);

/*---------------------------------------------------------------------------*/

int  galaxy_send_create(const char *);
void galaxy_recv_create(void);

void galaxy_send_magn(int, float);
void galaxy_recv_magn(void);

void galaxy_delete(int);

/*---------------------------------------------------------------------------*/

#endif
