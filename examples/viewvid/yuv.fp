!!ARBfp1.0

PARAM d = { -0.0625, -0.5000, -0.5000,  0.0000 };
PARAM R = {  1.1640,  0.0000,  1.5960 };
PARAM G = {  1.1640, -0.3910, -0.8130 };
PARAM B = {  1.1640,  2.0180,  0.0000 };

TEMP yuv;

TEX yuv, fragment.texcoord[0], texture[0], RECT;
ADD yuv, yuv, d;

DP3 result.color.r, yuv, R;
DP3 result.color.g, yuv, G;
DP3 result.color.b, yuv, B;
MOV result.color.a, yuv.a;
#MOV result.color, yuv;

END
