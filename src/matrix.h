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

#define M_DEG(r) (180.0f * (r) / PI)
#define M_RAD(d) (PI * (d) / 180.0f)

/*---------------------------------------------------------------------------*/

void m_init(float[16]);
void m_copy(float[16], const float[16]);
void m_xpos(float[16], const float[16]);
void m_invt(float[16], const float[16]);
void m_mult(float[16], const float[16], const float[16]);

/*---------------------------------------------------------------------------*/

void m_xfrm(float[4], const float[16], const float[4]);
void m_pfrm(float[4], const float[16], const float[4]);

/*---------------------------------------------------------------------------*/

void m_xrot(float[16], float[16], float);
void m_yrot(float[16], float[16], float);
void m_zrot(float[16], float[16], float);

void m_rotat(float[16], float[16], const float[3], float);
void m_trans(float[16], float[16], const float[3]);
void m_scale(float[16], float[16], const float[3]);
void m_basis(float[16], float[16], const float[3],
                                   const float[3],
                                   const float[3]);

/*---------------------------------------------------------------------------*/

void v_normal(float[3], const float[3]);

void v_cross(float[3], const float[3], const float[3]);
void v_plane(float[4], const float[3], const float[3], const float[3]);

/*---------------------------------------------------------------------------*/

#endif
