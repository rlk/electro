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

/*---------------------------------------------------------------------------*/

void  init_buffer(void);
void  free_buffer(void);
void  sync_buffer(void);

/*---------------------------------------------------------------------------*/

void  pack_alloc(int, const void *);
void  pack_index(int);
void  pack_event(char);
void  pack_float(float);

/*---------------------------------------------------------------------------*/

int   unpack_count(void);

void *unpack_alloc(int);
int   unpack_index(void);
char  unpack_event(void);
float unpack_float(void);

/*---------------------------------------------------------------------------*/

