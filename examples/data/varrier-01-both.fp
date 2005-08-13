!!ARBfp1.0

PARAM scale  = program.env[0];
PARAM offset = { -1.0, -1.0, 0.0, 1.0 }; 

TEMP posn;
TEMP tex_r;
TEMP tex_g;
TEMP tex_b;
TEMP base;
TEMP line;

# Scale the fragment position to [-1,+1].  Discard depth.

MUL posn, fragment.position, scale;
ADD posn, posn, offset;

# Transform the fragment position, giving line screen texture coordinates.

DP4 tex_r.r, state.matrix.texture[1].row[0], posn;
DP4 tex_r.g, state.matrix.texture[1].row[1], posn;

DP4 tex_g.r, state.matrix.texture[2].row[0], posn;
DP4 tex_g.g, state.matrix.texture[2].row[1], posn;

DP4 tex_b.r, state.matrix.texture[3].row[0], posn;
DP4 tex_b.g, state.matrix.texture[3].row[1], posn;

# Modulate the base material and line screen values for each channel.

TEX base,   fragment.texcoord[0], texture[0], 2D;
TEX line.r, tex_r,                texture[1], 2D;
TEX line.g, tex_g,                texture[2], 2D;
TEX line.b, tex_b,                texture[3], 2D;

MUL base, base, fragment.color.primary;
ADD base, base, fragment.color.secondary;
MUL result.color,   base, line;
DP3 result.color.a, line, line;

END
