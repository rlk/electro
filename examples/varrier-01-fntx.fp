!!ARBfp1.0

PARAM cycle  = { 0.777777, 0.777777, 0.777777, 1.0 };

TEMP base;
TEMP line;
TEMP texc;
TEMP temp;

TEX base,   fragment.texcoord[0], texture[0], 2D;
#TXP line.r, fragment.texcoord[1], texture[1], 2D;
#TXP line.g, fragment.texcoord[2], texture[2], 2D;
#TXP line.b, fragment.texcoord[3], texture[3], 2D;
#DP3 line.a, line, line;

RCP temp.r, fragment.texcoord[1].w;
RCP temp.g, fragment.texcoord[2].w;
RCP temp.b, fragment.texcoord[3].w;

MUL texc.r, fragment.texcoord[1].x, temp.r;
MUL texc.g, fragment.texcoord[1].x, temp.g;
MUL texc.b, fragment.texcoord[1].x, temp.b;

FRC texc, texc;
SGE line, texc, cycle;
DP3 line.a, line, line;

MUL result.color, base, line;

END
