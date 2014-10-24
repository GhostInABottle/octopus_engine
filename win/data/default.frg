#version 110
uniform vec4 vColor;
uniform sampler2D colorMap;
uniform int ticks;
uniform vec2 vTexSize;
varying vec2 vVaryingTexCoords;
void main(void) {
    gl_FragColor = vColor * texture2D(colorMap, vVaryingTexCoords.st);
}