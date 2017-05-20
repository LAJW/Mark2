
#version 120

uniform sampler2D diffuse;

void main() {

	const bool useNormals = true;
	const bool useShadow = true;
	const vec4 lightColor = vec4(1, 1, 1, 1);
	const vec2 resolution = vec2(1920, 1080);

	vec2 position = gl_TexCoord[0].st;
	vec3 light = vec3(resolution / 2.0, 0.1);
	vec3 nColor = texture2D(diffuse, position).rgb;

	nColor.g = 1.0 - nColor.g;

	//normals need to be converted to [-1.0, 1.0] range and normalized
	vec3 normal = normalize(nColor * 2.0 - 1.0);

	//here we do a simple distance calculation
	vec3 deltaPos = vec3((light.xy - gl_FragCoord.xy) / resolution.xy, light.z);

	vec3 lightDir = normalize(deltaPos);
	float lambert = useNormals ? clamp(dot(normal, lightDir), 0.0, 1.0) : 1.0;

	// float d = sqrt(dot(deltaPos, deltaPos));
	// float att = useShadow ? 1.0 / ( attenuation.x + (attenuation.y*d) + (attenuation.z*d*d) ) : 1.0;

	vec3 result = lightColor.rgb * lambert /* * att */;

	if (lambert > 0.5) {
		gl_FragColor = vec4(lightColor.rgb, (lambert - 0.5) * 0.25);
	} else {
		gl_FragColor = vec4(0, 0, 0, pow((0.5 - lambert) * 2, 1));
	}
}
