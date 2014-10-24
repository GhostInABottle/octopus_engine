#version 110
uniform vec4 vColor;
uniform sampler2D colorMap;
uniform int ticks;
uniform vec2 vTexSize;
varying vec2 vVaryingTexCoords;

// A sine wave effect.
void main(void) {
    float speed = 0.005;
    float period = 15.0;
    float amplitude = 0.01;
	vec2 tc = vVaryingTexCoords + vec2(sin(float(ticks) * speed + period * vVaryingTexCoords.t) * amplitude, 0.0);
    gl_FragColor = vColor * texture2D(colorMap, tc);
}