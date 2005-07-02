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

#ifndef MATRIX_H
#define MATRIX_H

/*---------------------------------------------------------------------------*/

#define PI 3.14159265f

#define TO_DEG(r) (180.0f * (r) / PI)
#define TO_RAD(d) (PI * (d) / 180.0f)

#define DOT3(v, w) ((v)[0] * (w)[0] + \
                    (v)[1] * (w)[1] + \
                    (v)[2] * (w)[2])
#define DOT4(v, w) ((v)[0] * (w)[0] + \
                    (v)[1] * (w)[1] + \
                    (v)[2] * (w)[2] + \
                    (v)[3] * (w)[3])

/*---------------------------------------------------------------------------*/

void load_idt(float[16]);

void load_mat(float[16], const float[16]);
void load_xps(float[16], const float[16]);
void load_inv(float[16], const float[16]);

/*---------------------------------------------------------------------------*/

void load_xlt_mat(float[16], float, float, float);
void load_scl_mat(float[16], float, float, float);
void load_rot_mat(float[16], float, float, float, float);

void load_xlt_inv(float[16], float, float, float);
void load_scl_inv(float[16], float, float, float);
void load_rot_inv(float[16], float, float, float, float);

void mult_xlt_mat(float[16], float, float, float);
void mult_scl_mat(float[16], float, float, float);
void mult_rot_mat(float[16], float, float, float, float);

void mult_xlt_int(float[16], float, float, float);
void mult_scl_int(float[16], float, float, float);
void mult_rot_int(float[16], float, float, float, float);

/*---------------------------------------------------------------------------*/

void mult_mat_mat(float[16], const float[16], const float[16]);
void mult_mat_vec(float[4],  const float[16], const float[4]);
void mult_xps_vec(float[4],  const float[16], const float[4]);

/*---------------------------------------------------------------------------*/

void normalize(float[3]);

void cross(float[3], const float[3], const float[3]);
void plane(float[4], const float[3], const float[3], const float[3]);

/*---------------------------------------------------------------------------*/

#endif
