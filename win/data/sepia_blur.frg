#version 110
uniform vec4 vColor;
uniform sampler2D colorMap;
uniform int ticks;
uniform vec2 vTexSize;
varying vec2 vVaryingTexCoords;

// Stylized 'blur' with sepia tone.
void main(void) {
	vec2 offset = 3.0 * vec2(1.0 / vTexSize.x, 1.0 / vTexSize.y);
	vec2 tc = vVaryingTexCoords;
    vec4 sum = texture2D(colorMap, tc);
	sum += texture2D(colorMap, tc - offset);
	sum += texture2D(colorMap, vec2(tc.s - offset.s, tc.t + offset.t));
	sum += texture2D(colorMap, vec2(tc.s + offset.s, tc.t - offset.t));
	sum += texture2D(colorMap, tc + offset);
	vec4 color = vColor * sum * 0.2;
	gl_FragColor.r = dot(color.rgb, vec3(.393, .769, .189));
	gl_FragColor.g = dot(color.rgb, vec3(.349, .686, .168));
	gl_FragColor.b = dot(color.rgb, vec3(.272, .534, .131));
	gl_FragColor.a = color.a;
}