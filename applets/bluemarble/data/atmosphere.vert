uniform vec3 sunDir;

varying vec2 normal;
varying vec3 sun;

void main()
{
    gl_Position = ftransform();
    normal = (gl_ModelViewMatrix * gl_Vertex).xy;
    sun = (gl_ModelViewMatrix * vec4(sunDir, 0.0)).xyz;
}
