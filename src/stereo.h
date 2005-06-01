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

#ifndef STEREO_H
#define STEREO_H

/*---------------------------------------------------------------------------*/

#define STEREO_NONE       0
#define STEREO_QUAD       1
#define STEREO_RED_BLUE   2
#define STEREO_VARRIER_11 3
#define STEREO_VARRIER_33 4

int stereo_quad      (int, int, int);
int stereo_red_blue  (int, int, int);
int stereo_varrier_11(int, int, int);
int stereo_varrier_33(int, int, int);

/*---------------------------------------------------------------------------*/

#endif
