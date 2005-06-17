!!ARBfp1.0

PARAM third = { 0.333333, 0.0, 0.0, 0.0 };

TEMP pos_r;
TEMP pos_g;
TEMP pos_b;
TEMP tex_r;
TEMP tex_g;
TEMP tex_b;
TEMP base;
TEMP test;
TEMP line;

# Discard the depth information in the fragment position.

MOV pos_g.x, fragment.position.x;
MOV pos_g.y, fragment.position.y;
MOV pos_g.z, 0;
MOV pos_g.w, 1;

# Offset the red and blue positions one third of a pixel from the green.

SUB pos_r, pos_g, third;
ADD pos_b, pos_g, third;

# Transform the fragment positions, giving line screen texture coordinates.

DP4 tex_r.r, state.matrix.texture[1].row[0], pos_r;
DP4 tex_r.g, state.matrix.texture[1].row[1], pos_r;

DP4 tex_g.r, state.matrix.texture[2].row[0], pos_g;
DP4 tex_g.g, state.matrix.texture[2].row[1], pos_g;

DP4 tex_b.r, state.matrix.texture[3].row[0], pos_b;
DP4 tex_b.g, state.matrix.texture[3].row[1], pos_b;

# Modulate the base material and line screen values for each channel.

TEX base,   fragment.texcoord[0], texture[0], 2D;
TEX line.r, tex_r,                texture[1], 2D;
TEX line.g, tex_g,                texture[2], 2D;
TEX line.b, tex_b,                texture[3], 2D;

MUL result.color,   base, line;
DP3 result.color.a, line, line;

END
