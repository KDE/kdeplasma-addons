uniform sampler2D dayTexture;
uniform sampler2D nightTexture;
uniform vec3 sunDir;

varying vec3 pos;


void main()
{
    vec3 dayColor = texture2D(dayTexture, gl_TexCoord[0].xy).rgb;
    vec3 nightColor = texture2D(nightTexture, gl_TexCoord[0].xy).rgb * 0.5;

    float diffuse = clamp(dot(normalize(pos), sunDir), 0.0, 1.0);
    vec3 finalColor = dayColor * diffuse + nightColor * max(0.0, 0.8 - diffuse);

    gl_FragColor = vec4(finalColor, 1.0);
}
