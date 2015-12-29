#version 110
uniform vec4 vColor;
uniform vec4 vColorKey;
uniform sampler2D colorMap;
uniform int ticks;
uniform vec2 vTexSize;
varying vec2 vVaryingTexCoords;
void main(void) {
    vec4 vTexColor = texture2D(colorMap, vVaryingTexCoords.st);
    if (vColorKey.a > 0.0 && vTexColor == vColorKey) vTexColor.a = 0.0;
    gl_FragColor = vColor * vTexColor;
}