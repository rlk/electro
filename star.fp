!!ARBfp1.0

ATTRIB icol = fragment.color;
ATTRIB texc = fragment.texcoord;

PARAM  half = { 0.5, 0.5, 0.0, 1.0 };
PARAM  const = { -29.5562243, 2.7182818 };

TEMP   temp;
TEMP   foob;

OUTPUT ocol = result.color;

MUL    foob, icol, icol;
RCP    foob.r, foob.r;
RCP    foob.g, foob.g;
RCP    foob.b, foob.b;

SUB    temp, texc, half;
DP3    temp, temp, temp;
MUL    temp, temp, foob;
MUL    temp, temp, const.x;

POW    ocol.r, const.y, temp.r;
POW    ocol.g, const.y, temp.g;
POW    ocol.b, const.y, temp.b;

END