varying vec2 normal;
varying vec3 sun;


void main()
{
    float dist = 1.0 - length(normal / 5.65);
    if(dist < 0.0)
        discard;
    float strength = dist * 16.0;
    float diffuse = clamp(dot(normalize(vec3(normal, 0.0)), normalize(sun)), 0.0, 1.0);

    vec3 atmoColor = vec3(0.5, 0.5, 1.0);

    gl_FragColor = vec4(atmoColor, strength * diffuse);
}
