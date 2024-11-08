#type vertex
#version 460 core
layout(location = 0) in vec3 a_Position;

out vec3 v_Position;

uniform mat4 u_View;
uniform mat4 u_Projection;

void main() {
	v_Position = a_Position;
    gl_Position = u_Projection * u_View * vec4(a_Position, 1.0);
}

#type fragment
#version 460 core
out vec4 out_Color;

in vec3 v_Position;

uniform samplerCube u_gCubeMap;
uniform float u_SampleDelta = 0.025f;

const float PI = 3.14159265359f;

void main() {
	vec3 N = normalize(v_Position);
	vec3 Up = vec3(0.0f, 1.0f, 0.0f);
	vec3 R = normalize(cross(Up, N));
	Up = normalize(cross(N, R));

	vec3 irradiance = vec3(0.0f);
	int sampleCnt = 0;
	for (float phi = 0.0f; phi < PI * 2.0f; phi += u_SampleDelta) {
		for (float theta = 0.0f; theta < PI / 2.0f; theta += u_SampleDelta) {
			vec3 xyzT = vec3(sin(theta) * cos(phi), sin(theta) * sin(phi), cos(phi));
			vec3 xyz = xyzT.x * R + xyzT.y * Up + xyzT.z * N;

			irradiance += texture(u_gCubeMap, xyz).rgb * cos(theta) * sin(theta);
			sampleCnt += 1;
		}
	}
	irradiance = PI * irradiance / float(sampleCnt);
	out_Color = vec4(irradiance, 1.0f);
}