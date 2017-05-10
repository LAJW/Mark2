#version 120

uniform sampler2D diffuse;

void main() {
	
	vec2 position = gl_TexCoord[0].st;
	vec2 rel = vec2(0.5, 0.5) - position;
	float light = (1 - sqrt(rel.x * rel.x + rel.y * rel.y)) * 2 - 0.5;
	if (light > 1.0) {
		light = 1.0;
	}
	vec4 color = texture2D(diffuse, position) * light;
	gl_FragColor = color;
}