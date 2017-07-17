#version 120

/*
uniform sampler2D diffuse;

void main() {
	float a = gl_TexCoord[0].st.x * 2 * 3.14;
	vec2 dir = vec2(cos(a), sin(a));
	gl_FragColor = vec4(1, 1, 1, 1);
	for (float i = 0; i < 0.5; i += 0.002) {
		vec4 cur = texture2D(diffuse, vec2(0.5, 0.5) + dir * i);
		if (cur.r == 0 && cur.g == 0 && cur.b == 0) {
			gl_FragColor = vec4(i, i, i, 1);
			break;
		}
	}
}
*/

#define PI 3.14

//uniform values
uniform sampler2D diffuse;
uniform float shadow_res;

//alpha threshold for our occlusion map
const float THRESHOLD = 0.75;

void main(void) {
  float distance = 1.0;

  for (float y = 0.0; y < shadow_res; y += 1.0) {
		//rectangular to polar filter
		vec2 norm = vec2(gl_TexCoord[0].s, y / shadow_res) * 2.0 - 1.0;
		float theta = PI * 1.5 + norm.x * PI; 
		float r = (1.0 + norm.y) * 0.5;

		//coord which we will sample from occlude map
		vec2 coord = vec2(-r * sin(theta), -r * cos(theta))/2.0 + 0.5;

		//sample the occlusion map
		vec4 data = texture2D(diffuse, coord);

		//the current distance is how far from the top we've come
		float dst = y / shadow_res;

		//if we've hit an opaque fragment (occluder), then get new distance
		//if the new distance is below the current, then we'll use that for our ray
		float caster = (data.r == 0 && data.g == 0 && data.b == 0) ? 1 : 0;
		if (caster > THRESHOLD) {
			distance = min(distance, dst);
			//NOTE: we could probably use "break" or "return" here
		}
  } 
  gl_FragColor = vec4(vec3(distance), 1.0);
}