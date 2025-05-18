#type vertex
#version 460 core

#include "Common/UniformBufferObjects.glsl"

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec2 a_TexCoords;
layout(location = 3) in vec3 a_Tangent;
layout(location = 4) in vec4 a_BoneWeights;
layout(location = 5) in ivec4 a_BoneID;

uniform mat4 u_Model;
out vec2 v_TexCoords;

void main() {
	v_TexCoords = a_TexCoords;
	gl_Position = u_ProjView * u_Model * vec4(a_Position, 1.0f);
}


#type fragment
#version 460 core

layout(location = 0) out vec4 g_Color;

in vec2 v_TexCoords;
uniform bool u_HasAlbedo = false;
uniform vec3 u_RawAlbedo;
uniform sampler2D u_AlbedoMap;

void main() {
	vec3 color = u_HasAlbedo ? texture(u_AlbedoMap, v_TexCoords).rgb : vec3(1.0, 0.0, 0.0);
	g_Color = vec4(color, 1.0);
}