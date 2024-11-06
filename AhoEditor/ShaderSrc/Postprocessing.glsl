#type vertex
#version 460 core
layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec2 a_TexCoords;

out vec2 v_TexCoords;

void main() {
	gl_Position = vec4(a_Position, 1.0f);
	v_TexCoords = a_TexCoords;
}

#type fragment
#version 460 core
out vec4 out_Color;
in vec2 v_TexCoords;

uniform sampler2D u_Result;
uniform usampler2D u_gEntity;

uniform uint u_SelectedEntityID;
uniform bool u_IsEntityIDValid;

void main() {
	ivec2 texelPos = ivec2(gl_FragCoord.xy);
	uint texelValue = texelFetch(u_gEntity, texelPos, 0).r;
	if (!u_IsEntityIDValid || texelValue == u_SelectedEntityID) {
		out_Color = vec4(texelFetch(u_Result, texelPos, 0).rgb, 1.0f);
		return;
	}

	ivec2 textureSize = textureSize(u_gEntity, 0);

	bool isEdge = false;
	for (int i = -1; !isEdge && i <= 1; i += 2) {
		for (int j = -1; j <= 1; j += 2) {
			ivec2 nxtPos = ivec2(texelPos) + ivec2(i, j);
			if (nxtPos.x >= 0 && nxtPos.x < textureSize.x && nxtPos.y >= 0 && nxtPos.y < textureSize.y) {
				if (texelFetch(u_gEntity, nxtPos, 0).r == u_SelectedEntityID) {
					isEdge = true;
					break;
				}
			}
		}
	}

	out_Color = isEdge ? vec4(1.0f, 1.0f, 0.0f, 1.0f) : vec4(texelFetch(u_Result, texelPos, 0).rgb, 1.0f);
}