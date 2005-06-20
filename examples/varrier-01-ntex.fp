!!ARBfp1.0

PARAM scale  = program.env[0];
PARAM offset = { -1.0, -1.0, 0.0, 1.0 }; 
PARAM cycle  = { 0.777777, 0.777777, 0.777777, 1.0 };

TEMP posn;
TEMP tex_r;
TEMP tex_g;
TEMP tex_b;
TEMP texc;
TEMP base;
TEMP line;

# Scale the fragment position to [-1,+1].  Discard depth.

MUL posn, fragment.position, scale;
ADD posn, posn, offset;

# Transform the fragment position, giving line screen texture S coordinates.

DP4 texc.r, state.matrix.texture[1].row[0], posn;
DP4 texc.g, state.matrix.texture[2].row[0], posn;
DP4 texc.b, state.matrix.texture[3].row[0], posn;

# Determine the line screen value for each channel.

FRC texc, texc;
SGE line, texc, cycle;

# Combine the base material, texture, and line screen values for each channel.

TEX base, fragment.texcoord[0], texture[0], 2D;
MUL base, base, fragment.color.primary;
ADD base, base, fragment.color.secondary;
MUL result.color,   base, line;
DP3 result.color.a, line, line;

END
