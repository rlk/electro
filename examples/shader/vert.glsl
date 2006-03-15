
attribute vec3 tangent;
attribute vec3 bitangent;

varying   vec3 T;
varying   vec3 B;
varying   vec3 N;
varying   vec3 L;

void main()
{
    vec3 eye;
    vec3 tmp;

    T = normalize(gl_NormalMatrix * tangent);
    B = normalize(gl_NormalMatrix * bitangent);
    N = normalize(gl_NormalMatrix * gl_Normal);

    eye = vec3(gl_ModelViewMatrix * gl_Vertex);
    tmp = gl_LightSource[0].position.xyz - eye;

    L.x = dot(tmp, T);
    L.y = dot(tmp, B);
    L.z = dot(tmp, N);

    gl_TexCoord[0] = gl_MultiTexCoord0;
    gl_Position    = ftransform();
}