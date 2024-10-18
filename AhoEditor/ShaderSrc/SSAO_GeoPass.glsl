#type vertex
#version 460 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec2 a_TexCoords;
layout(location = 3) in vec3 a_Tangent;
layout(location = 4) in vec3 a_Bitangent;

layout(std140, binding = 0) uniform CameraData{
	vec4 u_LightPosition[4];
	vec4 u_LightColor[4];
	mat4 u_View;
	mat4 u_Projection;
	mat4 u_LightViewMatrix; // ortho * view
	vec4 u_ViewPosition;
	ivec4 info; // info[0]: light counts

};

out vec3 v_Position;
out vec3 v_Normal;
out vec2 v_TexCoords;

uniform sampler2D u_Normal;
uniform mat4 u_Model;

void main() {
	vec4 PosViewSpace = u_View * u_Model * vec4(a_Position, 1.0f);
	gl_Position = u_Projection * PosViewSpace;
	v_TexCoords = a_TexCoords;
	
	vec3 T = normalize(vec3(u_Model * vec4(a_Tangent, 0.0f)));
	vec3 N = normalize(vec3(u_Model * vec4(a_Normal, 0.0f)));
	vec3 B = normalize(cross(N, T));
	mat3 TBN = transpose(mat3(T, B, N));

	v_Position = viewPos.xyz;


	v_Normal = mat * a_Normal;
}

#type fragment
#version 460 core

layout(location = 0) out vec4 g_PositionDepth;
layout(location = 1) out vec3 g_Normal;
layout(location = 2) out vec4 g_AlbedoSpec;

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