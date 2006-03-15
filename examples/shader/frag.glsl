
uniform sampler2D colormap;
uniform sampler2D normalmap;
varying vec3      N;
varying vec3      L;

void main()
{
    vec4  C = vec4(0, 0, 0, 1);
    vec3  normal;
    vec3  light;
    vec4  cm =           texture2D(colormap,  gl_TexCoord[0].xy);
    vec3  nm = normalize(texture2D(normalmap, gl_TexCoord[0].xy).xyz * 2.0 - 1.0);

    float k;

    normal = normalize(N);
    light = normalize(L);

    k = max(dot(nm, light), 0.0);

    gl_FragColor.xyz = cm.xyz * k;
    gl_FragColor.a   = cm.a;
}
