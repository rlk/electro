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
TEMP line_r;
TEMP line_g;
TEMP line_b;

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
DP4 tex_r.b, state.matrix.texture[1].row[2], pos_r;
DP4 tex_r.a, state.matrix.texture[1].row[3], pos_r;

DP4 tex_g.r, state.matrix.texture[2].row[0], pos_g;
DP4 tex_g.g, state.matrix.texture[2].row[1], pos_g;
DP4 tex_g.b, state.matrix.texture[2].row[2], pos_g;
DP4 tex_g.a, state.matrix.texture[2].row[3], pos_g;

DP4 tex_b.r, state.matrix.texture[3].row[0], pos_b;
DP4 tex_b.g, state.matrix.texture[3].row[1], pos_b;
DP4 tex_b.b, state.matrix.texture[3].row[2], pos_b;
DP4 tex_b.a, state.matrix.texture[3].row[3], pos_b;

# Modulate the base material and line screen values for each channel.

TEX base,   fragment.texcoord[0], texture[0], 2D;
TEX line_r, tex_r,                texture[1], 2D;
TEX line_g, tex_g,                texture[2], 2D;
TEX line_b, tex_b,                texture[3], 2D;

# Compute a reasonable alpha value for the combined pixel.

MOV test.a, 0;
ADD test.a, test.a, line_r.a;
ADD test.a, test.a, line_g.a;
ADD test.a, test.a, line_b.a;

MUL result.color.r, base.r, line_r.a;
MUL result.color.g, base.g, line_g.a;
MUL result.color.b, base.b, line_b.a;
MUL result.color.a, base.a, test.a;

END
