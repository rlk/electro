!!ARBfp1.0

TEMP base;
TEMP line;

TEX base,   fragment.texcoord[0], texture[0], 2D;
TXP line.r, fragment.texcoord[1], texture[1], 2D;
TXP line.g, fragment.texcoord[2], texture[2], 2D;
TXP line.b, fragment.texcoord[3], texture[3], 2D;
DP3 line.a, line, line;

MUL result.color, base, line;

END
