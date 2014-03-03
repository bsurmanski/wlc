#version 120

uniform float tick;

varying vec2 texture_coordinate;
uniform sampler2D color_texture;

void main(void)
{
    vec4 col = texture2D(color_texture, texture_coordinate);
    //gl_FragColor += tick;
    if(col.b + tick < 0.5){
        col = vec4(1.0, 0.0, 0.0, 1.0);
    } else {
        col = vec4(0.0, 1.0, 1.0, 1.0);
    }
    gl_FragColor = col;
    //vec4 outcol = vec4(1.0f - col.g - col.b, 1.0f - col.r - col.b,
    //1.0f - col.r - col.g, 1.0f);
    //gl_FragColor = col;//(vec4(1.0f) - outcol);
}
