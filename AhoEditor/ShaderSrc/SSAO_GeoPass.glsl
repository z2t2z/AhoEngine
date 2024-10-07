#type vertex
#version 450 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec2 a_TexCoords;

out vec3 v_Position;
out vec3 v_Normal;
out vec2 v_TexCoords;

uniform mat4 u_View;
uniform mat4 u_Projection;
uniform mat4 u_Model;

void main() {
	vec4 viewPos = u_View * u_Model * vec4(a_Position, 1.0f);
	v_Position = viewPos.xyz;
	gl_Position = u_Projection * viewPos;
	v_TexCoords = a_TexCoords;

	mat3 mat = transpose(inverse(mat3(u_View * u_Model)));
	v_Normal = mat * a_Normal;
}

#type fragment
#version 450 core

layout (location = 0) out vec4 g_PositionDepth;
layout (location = 1) out vec3 g_Normal;
layout (location = 2) out vec4 g_AlbedoSpec;

in vec3 v_Position;
in vec3 v_Normal;
in vec2 v_TexCoords;

uniform float u_Near;
uniform float u_Far;

float GetLinearizedDepth(float depth) {
	float z = depth * 2.0 - 1.0;
	return (2.0 * u_Near * u_Far) / (u_Far + u_Near - z * (u_Far - u_Near));
}

void main() {
	g_PositionDepth.xyz = v_Position;
	g_PositionDepth.a = GetLinearizedDepth(gl_FragCoord.z);
	g_Normal = normalize(v_Normal);
	g_AlbedoSpec.rgb = vec3(0.95);
	// g_AlbedoSpec.a = /* some specular factor */
}