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

#include "frustum.h"
#include "node.h"
#include "star.h"

/*---------------------------------------------------------------------------*/

struct galaxy
{
    int    count;
    int    S_num;
    int    N_num;

    float  magnitude;
    float  bound[6];

    GLuint buffer;

    struct star *S;
    struct node *N;
};

/*---------------------------------------------------------------------------*/

int  init_galaxy(void);
void draw_galaxy(int, int, const struct frustum *, float);
int  pick_galaxy(int, int, const float[3], const float[3]);

void prep_galaxy(void);

/*---------------------------------------------------------------------------*/

int  send_create_galaxy(const char *);
void recv_create_galaxy(void);

void send_set_galaxy_magnitude(int, float);
void recv_set_galaxy_magnitude(void);

void clone_galaxy(int);
void delete_galaxy(int);

/*---------------------------------------------------------------------------*/

void get_star_position(int, int, float[3]);

/*---------------------------------------------------------------------------*/

#endif
