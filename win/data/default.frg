#version 110
uniform vec4 vColor;
uniform vec4 vColorKey;
uniform sampler2D colorMap;
uniform float brightness;
uniform float contrast;
varying vec2 vVaryingTexCoords;
void main(void) {
    vec4 vTexColor = texture2D(colorMap, vVaryingTexCoords.st);
    if (vColorKey.a > 0.0 && vTexColor == vColorKey) vTexColor.a = 0.0;
    vec4 pixelColor = vColor * vTexColor;
    pixelColor.rgb = (pixelColor.rgb - 0.5) * max(contrast, 0.0) + 0.5 + brightness;
    gl_FragColor = pixelColor;
}