#version 110
const vec4 lumCoeff = vec4(0.2125, 0.7154, 0.0721, 0.0);
uniform vec4 vColor;
uniform vec4 vColorKey;
uniform sampler2D colorMap;
uniform float brightness;
uniform float contrast;
uniform float saturation;
varying vec2 vVaryingTexCoords;
void main(void) {
    vec4 vTexColor = texture2D(colorMap, vVaryingTexCoords.st);
    if (vColorKey.a > 0.0 && vTexColor == vColorKey) vTexColor.a = 0.0;
    vec4 pixelColor = vColor * vTexColor;
    pixelColor.rgb = (pixelColor.rgb - 0.5) * max(contrast, 0.0) + 0.5 + brightness;
    vec4 saturationIntensity = vec4(dot(pixelColor, lumCoeff));
    gl_FragColor = mix(saturationIntensity, pixelColor, saturation);
}