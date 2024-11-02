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

uniform sampler2D u_HDR;

const vec2 invAtan = vec2(0.1591f, 0.3183f);

vec2 GetSphericalMapUVFromWorldDir(vec3 dir) {
	vec2 uv = vec2(atan(dir.z, dir.x), asin(dir.y));
	uv = uv * invAtan + 0.5f;
	return uv;
}

void main() {
	vec2 uv = GetSphericalMapUVFromWorldDir(normalize(v_Position));
	out_Color = vec4(texture(u_HDR, uv).rgb, 1.0f);
}