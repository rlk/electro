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

#ifndef NODE_H
#define NODE_H

/*---------------------------------------------------------------------------*/

struct node
{
    float k;

    int star0;
    int starc;
    int nodeL;
    int nodeR;
    int nodep;
};

/*---------------------------------------------------------------------------*/

void prep_L(void);
void prep_R(void);
void prep_p(void);

void prep_init(void);
void prep_sort(void);
void prep_file_hip(const char *);

void node_init(void);
void node_draw(float P[3], float V[4][4]);

/*---------------------------------------------------------------------------*/

#endif
