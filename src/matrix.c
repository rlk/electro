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

#include <math.h>

#include "matrix.h"

/*---------------------------------------------------------------------------*/
/* Matrix operations                                                         */

void m_init(float A[16])
{
    A[0] = 1.0f; A[4] = 0.0f; A[8]  = 0.0f; A[12] = 0.0f;
    A[1] = 0.0f; A[5] = 1.0f; A[9]  = 0.0f; A[13] = 0.0f;
    A[2] = 0.0f; A[6] = 0.0f; A[10] = 1.0f; A[14] = 0.0f;
    A[3] = 0.0f; A[7] = 0.0f; A[11] = 0.0f; A[15] = 1.0f;
}

void m_copy(float A[16], const float B[16])
{
    A[0] = B[0];  A[4] = B[4];  A[8]  = B[8];  A[12] = B[12];
    A[1] = B[1];  A[5] = B[5];  A[9]  = B[9];  A[13] = B[13];
    A[2] = B[2];  A[6] = B[6];  A[10] = B[10]; A[14] = B[14];
    A[3] = B[3];  A[7] = B[7];  A[11] = B[11]; A[15] = B[15];
}

void m_xpos(float A[16], const float B[16])
{
    A[0] = B[0];  A[4] = B[1];  A[8]  = B[2];  A[12] = B[3];
    A[1] = B[4];  A[5] = B[5];  A[9]  = B[6];  A[13] = B[7];
    A[2] = B[8];  A[6] = B[9];  A[10] = B[10]; A[14] = B[11];
    A[3] = B[12]; A[7] = B[13]; A[11] = B[14]; A[15] = B[15];
}

void m_mult(float A[16], const float B[16], const float C[16])
{
    float T[16];

    T[0]  = B[0] * C[0]  + B[4] * C[1]  + B[8]  * C[2]  + B[12] * C[3];
    T[1]  = B[1] * C[0]  + B[5] * C[1]  + B[9]  * C[2]  + B[13] * C[3];
    T[2]  = B[2] * C[0]  + B[6] * C[1]  + B[10] * C[2]  + B[14] * C[3];
    T[3]  = B[3] * C[0]  + B[7] * C[1]  + B[11] * C[2]  + B[15] * C[3];

    T[4]  = B[0] * C[4]  + B[4] * C[5]  + B[8]  * C[6]  + B[12] * C[7];
    T[5]  = B[1] * C[4]  + B[5] * C[5]  + B[9]  * C[6]  + B[13] * C[7];
    T[6]  = B[2] * C[4]  + B[6] * C[5]  + B[10] * C[6]  + B[14] * C[7];
    T[7]  = B[3] * C[4]  + B[7] * C[5]  + B[11] * C[6]  + B[15] * C[7];

    T[8]  = B[0] * C[8]  + B[4] * C[9]  + B[8]  * C[10] + B[12] * C[11];
    T[9]  = B[1] * C[8]  + B[5] * C[9]  + B[9]  * C[10] + B[13] * C[11];
    T[10] = B[2] * C[8]  + B[6] * C[9]  + B[10] * C[10] + B[14] * C[11];
    T[11] = B[3] * C[8]  + B[7] * C[9]  + B[11] * C[10] + B[15] * C[11];

    T[12] = B[0] * C[12] + B[4] * C[13] + B[8]  * C[14] + B[12] * C[15];
    T[13] = B[1] * C[12] + B[5] * C[13] + B[9]  * C[14] + B[13] * C[15];
    T[14] = B[2] * C[12] + B[6] * C[13] + B[10] * C[14] + B[14] * C[15];
    T[15] = B[3] * C[12] + B[7] * C[13] + B[11] * C[14] + B[15] * C[15];

    m_copy(A, T);
}

/*---------------------------------------------------------------------------*/

void m_invt(float I[16], const float M[16])
{
    float T[16];
    float d;

    T[0]  = +(M[5]  * (M[10] * M[15] - M[11] * M[14]) -
              M[9]  * (M[6]  * M[15] - M[7]  * M[14]) +
              M[13] * (M[6]  * M[11] - M[7]  * M[10]));
    T[1]  = -(M[4]  * (M[10] * M[15] - M[11] * M[14]) -
              M[8]  * (M[6]  * M[15] - M[7]  * M[14]) +
              M[12] * (M[6]  * M[11] - M[7]  * M[10]));
    T[2]  = +(M[4]  * (M[9]  * M[15] - M[11] * M[13]) -
              M[8]  * (M[5]  * M[15] - M[7]  * M[13]) +
              M[12] * (M[5]  * M[11] - M[7]  * M[9]));
    T[3]  = -(M[4]  * (M[9]  * M[14] - M[10] * M[13]) -
              M[8]  * (M[5]  * M[14] - M[6]  * M[13]) +
              M[12] * (M[5]  * M[10] - M[6]  * M[9]));

    T[4]  = -(M[1]  * (M[10] * M[15] - M[11] * M[14]) -
              M[9]  * (M[2]  * M[15] - M[3]  * M[14]) +
              M[13] * (M[2]  * M[11] - M[3]  * M[10]));
    T[5]  = +(M[0]  * (M[10] * M[15] - M[11] * M[14]) -
              M[8]  * (M[2]  * M[15] - M[3]  * M[14]) +
              M[12] * (M[2]  * M[11] - M[3]  * M[10]));
    T[6]  = -(M[0]  * (M[9]  * M[15] - M[11] * M[13]) -
              M[8]  * (M[1]  * M[15] - M[3]  * M[13]) +
              M[12] * (M[1]  * M[11] - M[3]  * M[9]));
    T[7]  = +(M[0]  * (M[9]  * M[14] - M[10] * M[13]) -
              M[8]  * (M[1]  * M[14] - M[2]  * M[13]) +
              M[12] * (M[1]  * M[10] - M[2]  * M[9]));

    T[8]  = +(M[1]  * (M[6]  * M[15] - M[7]  * M[14]) -
              M[5]  * (M[2]  * M[15] - M[3]  * M[14]) +
              M[13] * (M[2]  * M[7]  - M[3]  * M[6]));
    T[9]  = -(M[0]  * (M[6]  * M[15] - M[7]  * M[14]) -
              M[4]  * (M[2]  * M[15] - M[3]  * M[14]) +
              M[12] * (M[2]  * M[7]  - M[3]  * M[6]));
    T[10] = +(M[0]  * (M[5]  * M[15] - M[7]  * M[13]) -
              M[4]  * (M[1]  * M[15] - M[3]  * M[13]) +
              M[12] * (M[1]  * M[7]  - M[3]  * M[5]));
    T[11] = -(M[0]  * (M[5]  * M[14] - M[6]  * M[13]) -
              M[4]  * (M[1]  * M[14] - M[2]  * M[13]) +
              M[12] * (M[1]  * M[6]  - M[2]  * M[5]));

    T[12] = -(M[1]  * (M[6]  * M[11] - M[7]  * M[10]) -
              M[5]  * (M[2]  * M[11] - M[3]  * M[10]) +
              M[9]  * (M[2]  * M[7]  - M[3]  * M[6]));
    T[13] = +(M[0]  * (M[6]  * M[11] - M[7]  * M[10]) -
              M[4]  * (M[2]  * M[11] - M[3]  * M[10]) +
              M[8]  * (M[2]  * M[7]  - M[3]  * M[6]));
    T[14] = -(M[0]  * (M[5]  * M[11] - M[7]  * M[9])  -
              M[4]  * (M[1]  * M[11] - M[3]  * M[9])  +
              M[8]  * (M[1]  * M[7]  - M[3]  * M[5]));
    T[15] = +(M[0]  * (M[5]  * M[10] - M[6]  * M[9])  -
              M[4]  * (M[1]  * M[10] - M[2]  * M[9])  +
              M[8]  * (M[1]  * M[6]  - M[2]  * M[5]));

    d = M[0] * T[0] + M[4] * T[4] + M[8] * T[8] + M[12] * T[12];

    I[0]  = T[0]  / d;
    I[1]  = T[4]  / d;
    I[2]  = T[8]  / d;
    I[3]  = T[12] / d;
    I[4]  = T[1]  / d;
    I[5]  = T[5]  / d;
    I[6]  = T[9]  / d;
    I[7]  = T[13] / d;
    I[8]  = T[2]  / d;
    I[9]  = T[6]  / d;
    I[10] = T[10] / d;
    I[11] = T[14] / d;
    I[12] = T[3]  / d;
    I[13] = T[7]  / d;
    I[14] = T[11] / d;
    I[15] = T[15] / d;
}

/*---------------------------------------------------------------------------*/
/* Vector transformers                                                       */

void m_xfrm(float v[4], const float A[16], const float u[4])
{
    v[0] = A[0] * u[0] + A[4] * u[1] + A[8]  * u[2] + A[12] * u[3];
    v[1] = A[1] * u[0] + A[5] * u[1] + A[9]  * u[2] + A[13] * u[3];
    v[2] = A[2] * u[0] + A[6] * u[1] + A[10] * u[2] + A[14] * u[3];
    v[3] = A[3] * u[0] + A[7] * u[1] + A[11] * u[2] + A[15] * u[3];
}

void m_pfrm(float v[4], const float A[16], const float u[4])
{
    v[0] = A[0]  * u[0] + A[1]  * u[1] + A[2]  * u[2] + A[3]  * u[3];
    v[1] = A[4]  * u[0] + A[5]  * u[1] + A[6]  * u[2] + A[7]  * u[3];
    v[2] = A[8]  * u[0] + A[9]  * u[1] + A[10] * u[2] + A[11] * u[3];
    v[3] = A[12] * u[0] + A[13] * u[1] + A[14] * u[2] + A[15] * u[3];
}

/*---------------------------------------------------------------------------*/
/* Matrix compositors                                                        */

void m_trns(float M[16], float I[16], float x, float y, float z)
{
    float A[16];
    float B[16];

    A[0] = 1.0f; A[4] = 0.0f; A[8]  = 0.0f; A[12] =    x;
    A[1] = 0.0f; A[5] = 1.0f; A[9]  = 0.0f; A[13] =    y;
    A[2] = 0.0f; A[6] = 0.0f; A[10] = 1.0f; A[14] =    z;
    A[3] = 0.0f; A[7] = 0.0f; A[11] = 0.0f; A[15] = 1.0f;

    B[0] = 1.0f; B[4] = 0.0f; B[8]  = 0.0f; B[12] =   -x;
    B[1] = 0.0f; B[5] = 1.0f; B[9]  = 0.0f; B[13] =   -y;
    B[2] = 0.0f; B[6] = 0.0f; B[10] = 1.0f; B[14] =   -z;
    B[3] = 0.0f; B[7] = 0.0f; B[11] = 0.0f; B[15] = 1.0f;

    m_mult(M, M, A);
    m_mult(I, B, I);
}

void m_xrot(float M[16], float I[16], float a)
{
    const float s = (float) sin((double) M_RAD(a));
    const float c = (float) cos((double) M_RAD(a));

    float A[16];
    float B[16];

    A[0] = 1.0f; A[4] = 0.0f; A[8]  = 0.0f; A[12] = 0.0f;
    A[1] = 0.0f; A[5] =    c; A[9]  =   -s; A[13] = 0.0f;
    A[2] = 0.0f; A[6] =    s; A[10] =    c; A[14] = 0.0f;
    A[3] = 0.0f; A[7] = 0.0f; A[11] = 0.0f; A[15] = 1.0f;

    m_xpos(B, A);
    m_mult(M, M, A);
    m_mult(I, B, I);
}

void m_yrot(float M[16], float I[16], float a)
{
    const float s = (float) sin((double) M_RAD(a));
    const float c = (float) cos((double) M_RAD(a));

    float A[16];
    float B[16];

    A[0] =    c; A[4] = 0.0f; A[8]  =    s; A[12] = 0.0f;
    A[1] = 0.0f; A[5] = 1.0f; A[9]  = 0.0f; A[13] = 0.0f;
    A[2] =   -s; A[6] = 0.0f; A[10] =    c; A[14] = 0.0f;
    A[3] = 0.0f; A[7] = 0.0f; A[11] = 0.0f; A[15] = 1.0f;

    m_xpos(B, A);
    m_mult(M, M, A);
    m_mult(I, B, I);
}

void m_zrot(float M[16], float I[16], float a)
{
    const float s = (float) sin((double) M_RAD(a));
    const float c = (float) cos((double) M_RAD(a));

    float A[16];
    float B[16];

    A[0] =    c; A[4] =   -s; A[8]  = 0.0f; A[12] = 0.0f;
    A[1] =    s; A[5] =    c; A[9]  = 0.0f; A[13] = 0.0f;
    A[2] = 0.0f; A[6] = 0.0f; A[10] = 1.0f; A[14] = 0.0f;
    A[3] = 0.0f; A[7] = 0.0f; A[11] = 0.0f; A[15] = 1.0f;

    m_xpos(B, A);
    m_mult(M, M, A);
    m_mult(I, B, I);
}

void m_scal(float M[16], float I[16], float x, float y, float z)
{
    float A[16];
    float B[16];

    A[0] =     x; A[4] =  0.0f; A[8]  =  0.0f; A[12] = 0.0f;
    A[1] =  0.0f; A[5] =     y; A[9]  =  0.0f; A[13] = 0.0f;
    A[2] =  0.0f; A[6] =  0.0f; A[10] =     z; A[14] = 0.0f;
    A[3] =  0.0f; A[7] =  0.0f; A[11] =  0.0f; A[15] = 1.0f;

    B[0] = 1.0f / x; B[4] =      0.0f; B[8]  =      0.0f; B[12] = 0.0f;
    B[1] =     0.0f; B[5] =  1.0f / y; B[9]  =      0.0f; B[13] = 0.0f;
    B[2] =     0.0f; B[6] =      0.0f; B[10] =  1.0f / z; B[14] = 0.0f;
    B[3] =     0.0f; B[7] =      0.0f; B[11] =      0.0f; B[15] = 1.0f;

    m_mult(M, M, A);
    m_mult(I, B, I);
}

/*---------------------------------------------------------------------------*/

void v_cross(float u[3], const float v[3], const float w[3])
{
    u[0] = v[1] * w[2] - v[2] * w[1];
    u[1] = v[2] * w[0] - v[0] * w[2];
    u[2] = v[0] * w[1] - v[1] * w[0];
}

/*---------------------------------------------------------------------------*/
