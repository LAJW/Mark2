#version 120

/*

uniform sampler2D diffuse;

void main() {
	vec2 pos = gl_TexCoord[0].st;
	vec2 rel = pos - vec2(0.5, 0.5);
	float dist = sqrt(rel.x * rel.x + rel.y * rel.y);
	float a = atan(rel.x, rel.y) / 3.14 / 2;
	if (a < 0) { 
		a = 1 - a;
	}
	float val = texture2D(diffuse, vec2(a, 0)).r;
	if (dist > val) {
		gl_FragColor = vec4(0, 0, 0, 0.2);
	} else {
		gl_FragColor = vec4(0, 0, 0, 0);
	}
}

*/

#define PI 3.14

//uniform values
uniform sampler2D diffuse;
const vec2 resolution = vec2(512, 512);

//sample from the 1D distance map
float sample(vec2 coord, float r) {
	return step(r, texture2D(diffuse, coord).r);
}

void main(void) {
	//rectangular to polar
	vec2 norm = gl_TexCoord[0].st * 2.0 - 1.0;
	float theta = -atan(norm.y, norm.x);
	float r = length(norm); 
	float coord = (theta + PI) / (2.0*PI);

	//the tex coord to sample our 1D lookup texture 
	//always 0.0 on y axis
	vec2 tc = vec2(coord, 0.0);

	//the center tex coord, which gives us hard shadows
	float center = sample(tc, r);        

	//we multiply the blur amount by our distance from center
	//this leads to more blurriness as the shadow "fades away"
	float blur = (1./resolution.x)  * smoothstep(0., 1., r); 

	//now we use a simple gaussian blur
	float sum = 0.0;

	sum += sample(vec2(tc.x - 4.0*blur, tc.y), r) * 0.05;
	sum += sample(vec2(tc.x - 3.0*blur, tc.y), r) * 0.09;
	sum += sample(vec2(tc.x - 2.0*blur, tc.y), r) * 0.12;
	sum += sample(vec2(tc.x - 1.0*blur, tc.y), r) * 0.15;

	sum += center * 0.16;

	sum += sample(vec2(tc.x + 1.0*blur, tc.y), r) * 0.15;
	sum += sample(vec2(tc.x + 2.0*blur, tc.y), r) * 0.12;
	sum += sample(vec2(tc.x + 3.0*blur, tc.y), r) * 0.09;
	sum += sample(vec2(tc.x + 4.0*blur, tc.y), r) * 0.05;

	//sum of 1.0 -> in light, 0.0 -> in shadow

	//multiply the summed amount by our distance, which gives us a radial falloff
	//then multiply by vertex (light) color  
	gl_FragColor = gl_Color * vec4(vec3(0.0), pow(1 - sum * smoothstep(1.0, 0.0, r), 3));
}